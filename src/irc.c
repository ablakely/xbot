/*
 * irc.c: IRC connection and parser
 * xbot: an advanced IRC bot for *nix and Windows
 *
 * This file contains the functions for connecting to an IRC server, sending
 * and receiving data, and parsing the data received from the server.
 *
 * Written by Aaron Blakely <aaron@ephasic.org>
**/

#include "irc.h"
#include "util.h"
#include "events.h"
#include "module.h"
#include "channel.h"
#include "logger.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#ifdef _WIN32
#define FDOPEN _fdopen
#define SETBUF setbuf
#else
#include <stdbool.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#define FDOPEN fdopen
#define SETBUF setbuf
#endif

void irc_connect(struct irc_conn *bot)
{
#ifdef _WIN32
    char titlebuf[256];
    WSADATA wsaData;
	struct sockaddr_in server;
    struct hostent *host;

    sprintf(titlebuf, "xbot [connecting]: %s:%s", bot->host, bot->port);
    SetConsoleTitle(titlebuf);

    if (bot->use_ssl)
        ssl_init();

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        eprint("WSAStartup failed.\n");
        exit(EXIT_FAILURE);
    }

	bot->srv_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (bot->srv_fd == INVALID_SOCKET)
	{
		eprint("Error creating socket: %d\n", WSAGetLastError());
		WSACleanup();

		return;
	}

	memset(&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(bot->host);
	server.sin_port = htons(atoi(bot->port));

    // resolve hostname
    if (server.sin_addr.s_addr == INADDR_NONE)
    {
        host = gethostbyname(bot->host);
        if (host == NULL)
        {
            eprint("Error resolving hostname: %d\n", WSAGetLastError());
            closesocket(bot->srv_fd);
            WSACleanup();

            return;
        }

        memcpy(&server.sin_addr, host->h_addr_list[0], host->h_length);
    }

	if (connect(bot->srv_fd, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
	{
		eprint("Failed to connect to IRC server: %d\n", WSAGetLastError());

		closesocket(bot->srv_fd);
		WSACleanup();

		return;
	}


    if (bot->use_ssl)
        ssl_connect();

    sprintf(titlebuf, "xbot [connected]: %s:%s", bot->host, bot->port);
    SetConsoleTitle(titlebuf);
#else
    struct addrinfo hints;
    struct addrinfo *res, *r;

    if (bot->use_ssl)
        ssl_init();

    memset(&hints, 0, sizeof hints);
    hints.ai_family		= AF_UNSPEC;
    hints.ai_socktype	= SOCK_STREAM;

    if (getaddrinfo(bot->host, bot->port, &hints, &res) != 0)
    {
        eprint("Error: Cannot resolve hostname '%s':", bot->host);
    }

    for (r = res; r; r->ai_next)
    {
        if ((bot->srv_fdi = socket(r->ai_family, r->ai_socktype, r->ai_protocol)) == -1)
        {
            continue;
        }

        if (connect(bot->srv_fdi, r->ai_addr, r->ai_addrlen) == 0)
        {
            break;
        }

        close(bot->srv_fdi);
    }

    freeaddrinfo(res);
    if (!r)
    {
        eprint("[IRC] Error: Cannot connect to host '%s'\n", bot->host);
    }


    if (bot->use_ssl)
        ssl_connect();

    bot->srv_fd = FDOPEN(bot->srv_fdi, "r+");
    xlog("[IRC] Connected!\n");
#endif
}

void irc_auth(struct irc_conn *bot)
{
    irc_raw(bot, "NICK %s", bot->nick);
    irc_raw(bot, "USER %s \" %s :%s", bot->user, bot->host, bot->real_name);

#ifndef _WIN32
    if (!bot->use_ssl)
    {
        fflush(bot->srv_fd);
        SETBUF(bot->srv_fd, NULL);
    }
#endif
}

void set_ssl_init(struct irc_conn *bot, void *func)
{
    bot->sslmod_init = func;
}

void set_ssl_connect(struct irc_conn *bot, void *func)
{
    bot->sslmod_connect = func;
}

void set_ssl_write(struct irc_conn *bot, void *func)
{
    bot->sslmod_write = func;
}

void set_ssl_read(struct irc_conn *bot, void *func)
{
    bot->sslmod_read = func;
}

void set_ssl_cleanup(struct irc_conn *bot, void *func)
{
    bot->sslmod_cleanup = func;
}

void set_ssl_get_fd(struct irc_conn *bot, void *func)
{
    bot->sslmod_get_fd = func;
}

void ssl_init()
{
    struct irc_conn *bot = get_bot();
    void *sslinit = bot->sslmod_init;

    ((void (*)())sslinit)();
}

void ssl_connect()
{
    struct irc_conn *bot = get_bot();
    void *sslconnect = bot->sslmod_connect;

    ((void (*)())sslconnect)();
}

int ssl_read(char *buf, int len)
{
    struct irc_conn *bot = get_bot();
    void *sslread = bot->sslmod_read;

    return ((int (*)(char *, int))sslread)(buf, len);
}

int ssl_write(char *buf, int len)
{
    struct irc_conn *bot = get_bot();
    void *sslwrite = bot->sslmod_write;

    return ((int (*)(char *, int))sslwrite)(buf, len);
}

void ssl_cleanup()
{
    struct irc_conn *bot = get_bot();
    void *sslcleanup = bot->sslmod_cleanup;

    ((void (*)())sslcleanup)();
}

int ssl_get_fd()
{
    struct irc_conn *bot = get_bot();
    void *sslgetfd = bot->sslmod_get_fd;

    return ((int (*)())sslgetfd)();
}

void irc_notice(struct irc_conn *bot, char *to, char *fmt, ...)
{
    char msg_[4096];
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(msg_, sizeof msg_, fmt, ap);
    va_end(ap);

    irc_raw(bot, "NOTICE %s :%s", to, msg_);
}

void irc_privmsg(struct irc_conn *bot, char *to, char *fmt, ...)
{
    char msg_[4096];
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(msg_, sizeof msg_, fmt, ap);
    va_end(ap);

    irc_raw(bot, "PRIVMSG %s :%s", to, msg_);
}

void irc_raw(struct irc_conn *bot, char *fmt, ...)
{
    va_list ap;
	char outbuf[4096];
    char *p;

    va_start(ap, fmt);
    vsnprintf(bot->out, OUTBUF_SIZE, fmt, ap);
    va_end(ap);

	sprintf(outbuf, "%s\r\n", bot->out);
    printf("<< %s\n", outbuf);
    if (bot->use_ssl)
    {
        sprintf(outbuf, "%s\r\n", bot->out);
        if (ssl_write(outbuf, strlen(outbuf)) <= 0)
        {
            eprint("Error: Cannot write to SSL server\n");
        }
    }
    else
    {
#ifdef _WIN32
        sprintf(outbuf, "%s\r\n", bot->out);
        send(bot->srv_fd, outbuf, strlen(outbuf), 0);
#else
        fprintf(bot->srv_fd, "%s\r\n", bot->out);
#endif
    }
}


void irc_join(struct irc_conn *bot, char *chan)
{
    irc_raw(bot, "JOIN %s", chan);

    add_channel(chan);
}

void irc_part(struct irc_conn *bot, char *chan, char *reason)
{
    if (!reason)
    {
        reason = "";
    }

    irc_raw(bot, "PART %s :%s", chan, reason);
}

void irc_ban(struct irc_conn *bot, char *channel, char *user)
{
    char *host = get_user_host(user);
    char *un   = get_user_user(user);

    irc_raw(bot, "MODE %s +b *!%s@%s", channel, un, host);
}

void irc_kick(struct irc_conn *bot, char *channel, char *user, char *reason)
{
    if (!reason)
    {
        reason = "";
    }

    irc_raw(bot, "KICK %s %s :%s", channel, user, reason);
}

void irc_mode(struct irc_conn *bot, char *channel, char *mode)
{
    irc_raw(bot, "MODE %s %s", channel, mode);
}

void irc_ctcp(struct irc_conn *bot, char *to, char *fmt, ...)
{
    char msg_[4096];
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(msg_, sizeof msg_, fmt, ap);
    va_end(ap);

    irc_privmsg(bot, to, "\001%s\001", msg_);
}

void irc_parse_raw(struct irc_conn *bot, char *raw)
{
    char *user, *host, *par, *text, *chan, *nick, *nicks, *tmp;
    char set = ' ';
    char *buf = NULL;
    char *mode = NULL;
    char *modestr;
    char *arg = NULL;
    char **args = NULL;

    int i, j, k;

    int modecount = 0;
    int argcount = 0;
    int space_count = 0;
    int tmpcount = 0;
    user = bot->host;

    if (!raw || !*raw)
    {
        return;
    }

    if (raw[0] == ':')
    {
        user = raw + 1;
        raw  = skip(user, ' ');

        if (raw[0] == '\0')
        {
            return;
        }

        host = skip(user, '!');
    }

    skip(raw, '\r');
    par  = skip(raw, ' ');
    text = skip(par, ':');
    
    trim(par);

	printf("dbug raw: %s\r\n", raw);

    fire_handler(bot, RAW, raw);

    if (!strcmp("PONG", raw))
    {
        return;
    }

    if (!strcmp("PRIVMSG", raw))
    {
        // check for CTCP
        if (text[0] == '\001')
        {
            char *ctcp = text + 1;
            char *end  = strchr(ctcp, '\001');

            if (end)
            {
                *end = '\0';

                // reply to version request
                if (!strcmp("VERSION", ctcp))
                {
#ifdef _WIN32
                    irc_notice(bot, user, "VERSION xbot: v%s [Windows] (https://github.com/ablakely/xbot)", VERSION);
#else
                    irc_notice(bot, user, "VERSION xbot: v%s [Linux] (https://github.com/ablakely/xbot)", VERSION);
#endif
                }
                else
                {
                    fire_handler(bot, CTCP, user, host, par, ctcp);
                }
            }
        }
        else
        {
            if (!strcmp(par, bot->nick))
            {
                fire_handler(bot, PRIVMSG_SELF, user, host, text);
            }
            else
            {
                fire_handler(bot, PRIVMSG_CHAN, user, host, par, text);
            }
        }
    }
    else if (!strcmp("JOIN", raw))
    {
        if (!strcmp(user, bot->nick))
        {
            add_channel(text);
            add_user_to_channel(user, host, text);
            irc_raw(bot, "NAMES %s", text);

            fire_handler(bot, JOIN_MYSELF, user, host, text);
        }
        else
        {
            add_user_to_channel(user, host, text);
            fire_handler(bot, JOIN, user, host, text);
        }
    }
    else if (!strcmp("PART", raw))
    {
        if (!strcmp(user, bot->nick))
        {
            remove_channel(text);
            fire_handler(bot, PART_MYSELF, user, host, text);
        }
        else
        {
            remove_user_from_channel(user, text);
            fire_handler(bot, PART, user, host, text);
        }

    }
    else if (!strcmp("QUIT", raw))
    {
        user_quit(user);
        fire_handler(bot, QUIT, user, host, text);
    }
    else if (!strcmp("PING", raw))
    {
        irc_raw(bot, "PONG %s", text);
    }
    else if (!strcmp("001", raw))
    {
        for (i = 0; i < bot->cfchan_count; i++)
        {
            irc_join(bot, bot->cfchannels[i]);
        }

        fire_handler(bot, IRC_CONNECTED, text);
    }
    else if (!strcmp("433", raw))
    {
        eprint("Error: Nickname '%s' is already in use\n", bot->nick);

        fire_handler(bot, NICK_INUSE, text);

#ifdef _WIN32
        _snprintf(bot->nick, sizeof bot->nick, "%s_", bot->nick);
#else
        sprintf(bot->nick, "%s_", bot->nick);
#endif
        irc_raw(bot, "NICK %s", bot->nick);
    }
    else if (!strcmp("353", raw))
    {
        chan = skip(par, ' ');
        chan = skip(chan, '=');
        chan = skip(chan, ' ');

#ifdef _WIN32
        nicks = _strdup(text);
#else
        nicks = strdup(text);
#endif
        nick  = strtok(nicks, " ");

        while (nick)
        {
            tmp = nick;
            if (nick[0] == '@' || nick[0] == '+' || nick[0] == '%' || nick[0] == '~' || nick[0] == '&')
            {
                tmp++;
            }

            if (get_user(tmp))
            {
                nick = strtok(NULL, " ");
                continue;
            }

            add_user_to_channel(nick, "", chan);

            irc_raw(bot, "WHO %s", tmp);
            nick = strtok(NULL, " ");
        }

        fire_handler(bot, IRC_NAMREPLY, chan, text);
    }
    else if (!strcmp(raw, "352"))
    {
        char *chan, *user, *host, *server, *nick, *flags, *realname;

        chan = skip(par, ' ');
        user = skip(chan, ' ');
        host = skip(user, ' ');
        server = skip(host, ' ');
        nick = skip(server, ' ');
        flags = skip(nick, ' ');
        realname = skip(text, ' ');

        update_user(nick, user);
        update_host(nick, host);
        update_server(nick, server);
        update_realname(nick, realname);

        fire_handler(bot, IRC_WHOREPLY, chan, user, host, server, nick, flags, realname);
    }
    else if (!strcmp("NICK", raw) && !strcmp(user, bot->nick))
    {
            strlcpy(bot->nick, text, sizeof bot->nick);
            fire_handler(bot, NICK_MYSELF, user, text);
    }
    else if (!strcmp("MODE", raw))
    {
        printf("dbug mode: %s %s %s\n", user, par, text);
        // dbug mode: Yin #lobby +qo ABx2 ABx2

        fire_handler(bot, MODE, user, par, text);

        // split the mode string into individual modes
        // and fire the MODE_PARSED handler for each mode
        // example:
        //   if the mode from IRC is
        //   MODE #lobby +qo ABx2 ABx2
        //
        //   the MODE_PARSED handler will be fired 2 times
        //   with the following arguments:
        //   user: Yin
        //   set: +
        //   mode: q
        //   arg: ABx2
        //
        //   TODO: modes set by the server are not parsed correctly
        //   use the MODE handler to parse modes set by the server


        buf = skip(par, ' ');
        tmp = malloc(strlen(text) + 1);
        args = malloc(strlen(text) * sizeof(char *));
        modestr = malloc(strlen(text) + strlen(par) + 1);

        memset(tmp, 0, strlen(text) + 1);

        sprintf(modestr, "%s %s", buf, text);

        for (i = 0; i < strlen(modestr); i++)
        {

            if (modestr[i] == ' ')
            {
                space_count++;
                continue;
            }

            if (space_count == 0 && (modestr[i] == '+' || modestr[i] == '-'))
            {
                buf = strdup(modestr + i);
                
                set = buf[0];

                free(buf);
                continue;
            }

            if (space_count == 0 && modestr[i] != '+' && modestr[i] != '-' && !mode)
            {
                mode = modestr + i;

                while (modestr[i] != ' ' && modestr[i] != '\0')
                {
                    tmpcount++;
                    i++;
                }

                mode = malloc(tmpcount + 1);

                for (j = 0; j < tmpcount; j++)
                {
                    mode[j] = modestr[i - tmpcount + j];
                    modecount++;
                }

                mode[tmpcount] = '\0';

                i -= tmpcount;

                continue;
            }

            if (space_count >= 1 && arg == NULL)
            {
                arg = (char *)malloc(strlen(modestr + i) + 1);

                strlcpy(arg, modestr + i, strlen(modestr + i) + 1);

                // split arg into individual args separated by spaces and store them in args

                k = 0;
                for (j = 0; j < strlen(arg) + 1; j++)
                {
                    if (arg[j] == ' ' || arg[j] == '\0')
                    {
                        args[argcount] = malloc(strlen(tmp) + 1);
                        memset(args[argcount], 0, strlen(tmp) + 1);

                        strlcpy(args[argcount], tmp, strlen(tmp) + 1);

                        argcount++;
                        k = 0;
                    }
                    else
                    {
                        memcpy(tmp + k, arg + j, 1);

                        k++;
                    }
                }

            }
        }

        for (i = 0; i < modecount; i++)
        {
            if (argcount >= i)
            {
                printf("dbug mode parsed: %s %s %c %c %s\n", user, par, set, mode[i], args[i]);
                fire_handler(bot, MODE_PARSED, user, par, set, mode[i], args[i]);

                set_channel_mode(par, args[i], set, mode[i]);
            }
            else
            {
                printf("dbug mode parsed: %s %s %c %c\n", user, par, set, mode[i]);
                fire_handler(bot, MODE_PARSED, user, par, set, mode[i], NULL);
            }
        }

        free(tmp);
        free(args);
        free(mode);
        free(modestr);
    }

    if (arg != NULL)
    {
        free(arg);
    }
}

#ifdef _WIN32
BOOL check_hostmask_match(char *mask, char *host)
#else
bool check_hostmask_match(char *mask, char *host)
#endif
{
    char *m = mask;
    char *h = host;

    while (*m && *h)
    {
        if (*m == '*')
        {
            m++;
            if (!*m)
            {
                return 1;
            }

            while (*h && *h != *m)
            {
                h++;
            }
        }
        else if (*m == *h)
        {
            m++;
            h++;
        }
        else
        {
            return false;
        }
    }

    return true;
}
