#define MY_DLL_EXPORTS 1

#include "util.h"
#include "irc.h"
#include "events.h"
#include "module.h"
#include "timers.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct said_hi
{
    int timer_id;
    int repeat;

    char user[50];
    char host[150];
    char chan[70];
};

MY_API void said_h(struct irc_conn *bot, void *data)
{
    struct said_hi *hi = (struct said_hi *)data;

    irc_privmsg(bot, hi->chan, "[timer %d] %d : %s", hi->timer_id, get_timer_repeat(hi->timer_id), hi->user);

    if ((get_timer_repeat(hi->timer_id) + 1) > hi->repeat)
    {
        free(hi);
    }
}

MY_API void hello(struct irc_conn *bot, char *user, char *host, char *chan, const char *text)
{
    struct said_hi *hi;

	char *buf = (char *)malloc(sizeof(char *) * 500);
	sprintf(buf, "hi %s", bot->nick);

	if (!strcmp(text, buf))
	{
        hi = calloc(1, sizeof(struct said_hi));

        irc_privmsg(bot, chan, "hi %s", user);

        sprintf(hi->user, "%s", user);
        sprintf(hi->host, "%s", host);
        sprintf(hi->chan, "%s", chan);

        hi->repeat = 3;
        hi->timer_id = add_timer(bot, 10, hi->repeat, said_h, hi);
	}

	free(buf);
}


MY_API void hello_join(struct irc_conn *bot, char *user, char *host, char *chan)
{
    printf("%s!%s joined %s\n", user, host, chan);

    irc_privmsg(bot, chan, "Hi %s! Welcome to %s", user, chan);
}

MY_API void mod_init()
{
    register_module("hello", "Aaron Blakely", "v0.05", "Test module");
    add_handler(PRIVMSG_CHAN, hello);
    add_handler(JOIN, hello_join);
}

MY_API void mod_unload()
{
    unregister_module("hello");
    del_handler(PRIVMSG_CHAN, hello);
    del_handler(JOIN, hello_join);
}
