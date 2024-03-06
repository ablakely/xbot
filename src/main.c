/*
 * xbot: Just another IRC bot
 *
 * Written by Aaron Blakely <aaron@ephasic.org>
**/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>

#include "config.h"
#include "irc.h"
#include "db.h"
#include "util.h"
#include "events.h"
#include "module.h"
#include "channel.h"
#include "timers.h"

#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/select.h>
#endif

static time_t trespond;

int main()
{
    int n;
    fd_set rd;
    struct irc_conn bot;
    struct timeval tv;
    struct timeval last_ping;


    char *p;
	int bytesRecv;

    bot.in = calloc(INBUF_SIZE, sizeof(char));
    bot.out = calloc(OUTBUF_SIZE, sizeof(char));

    last_ping.tv_sec = time(NULL);

    set_bot(&bot);

    init_events();
    init_timers();
    init_mods();
    // Read the config

    bot = read_config(bot, "xbot.cfg");

    // check if the db exists, if not, create it
    if (access(bot.db_file, F_OK) == -1)
    {
        printf("Creating database file: %s\n", bot.db_file);
        bot.db = (struct db_table *)malloc(sizeof(struct db_table));
        memset(bot.db, 0, sizeof(struct db_table));
        set_bot_db(bot.db);

        bot.db->count = 0;
        bot.db->hashes = NULL;

        db_write(bot.db, bot.db_file);
    }
    else
    {
        printf("Reading database file: %s\n", bot.db_file);
        bot.db = db_read(bot.db_file);
        set_bot_db(bot.db);
    }

    // run autoload
    run_autoload(&bot);

    // Connect to the server
    printf("Connecting to %s...\n", bot.host);

    irc_connect(&bot);
    trespond = time(NULL);

    irc_auth(&bot);

    for (;;)
    {
        fire_timers();
        fire_handler(&bot, TICK, NULL);

        FD_ZERO(&rd);

#ifdef _WIN32
		FD_SET(bot.srv_fd, &rd);
#else
		FD_SET(0, &rd);
        FD_SET(fileno(bot.srv_fd), &rd);
#endif
        tv.tv_sec  = 1;
        tv.tv_usec = 0;

#ifdef _WIN32
		n = select(bot.srv_fd, &rd, NULL, NULL, &tv);
		if (n == SOCKET_ERROR)
		{
			eprint("xbot: error on select(): %d\n", WSAGetLastError());
			closesocket(bot.srv_fd);
			WSACleanup();

			return -1;
		}
#else
        n = select(fileno(bot.srv_fd) + 1, &rd, 0, 0, &tv);
        if (n < 0)
		{
            if (errno == EINTR)
                continue;

            eprint("xbot: error on select()\n");
            return -1;
        }
#endif
        else if (n == 0)
        {
            if (time(NULL) - trespond >= 300)
            {
                eprint("xbot shutting down: parse timeout\n");
                return -1;
            }

            if (time(NULL) - last_ping.tv_sec >= 120)
            {
                last_ping.tv_sec = time(NULL);
                irc_raw(&bot, "PING %s", bot.host);
            }

            continue;
        }
#ifdef _WIN32
        if (FD_ISSET(bot.srv_fd, &rd))
		{
			bytesRecv = recv(bot.srv_fd, bot.in, INBUF_SIZE, 0);
			if (bytesRecv == SOCKET_ERROR)
			{
				eprint("Error receiving data: %d\n", WSAGetLastError());
				closesocket(bot.srv_fd);
				WSACleanup();

				return -1;
			}

            if (bytesRecv == 0)
            {
                eprint("xbot: remote host closed connection\n");
                return 0;
            }

			bot.in[bytesRecv] = '\0';

			printf("recv: %s\r\n", bot.in);

            // split bot.in into lines by \r\n and parse each one

            while (1)
            {
                // remove \r
                p = strchr(bot.in, '\r');
                p = strchr(bot.in, '\n');
                if (p == NULL)
                    break;

                *p = '\0';
                irc_parse_raw(&bot, bot.in);
                memmove(bot.in, p + 1, strlen(p + 1) + 1);
            }

            free(p);
#else
		if (FD_ISSET(fileno(bot.srv_fd), &rd))
		{
            if (fgets(bot.in, INBUF_SIZE, bot.srv_fd) == NULL)
            {
                eprint("xbot: remote host closed connection\n");
                return 0;
            }

			printf("recv: %s\r\n", bot.in);
            irc_parse_raw(&bot, bot.in);
#endif
            trespond = time(NULL);
        }

    }

    return 0;
}
