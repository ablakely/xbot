#include "channel.h"
#define MY_DLL_EXPORTS 1

#include "util.h"
#include "irc.h"
#include "events.h"
#include "module.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windef.h>
BOOL hasAccess(char *user, char *chan)
{
    if (is_op(chan, user) || is_halfop(chan, user))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
#else
#include <stdbool.h>
bool hasAccess(char *user, char *chan)
{
    if (is_op(chan, user) || is_halfop(chan, user))
    {
        return true;
    }
    else
    {
        return false;
    }
}
#endif

MY_API void chanop_privmsg_handler(struct irc_conn *bot, char *user, char *host, char *chan, const char *text)
{
    char *nick;
    char *buf = (char *)malloc(sizeof(char *) * 500);
    int sn = 1;

    if (strstr(text, "!kb") != NULL)
    {
        if (!hasAccess(user, chan))
            goto NO_ACCESS;

        nick = skip((char *)text, ' ');
        sprintf(buf, "Requested by %s", user);
        
        irc_ban(bot, chan, nick);
        irc_kick(bot, chan, nick, buf);
    }
    else if (strstr(text, "!op") != NULL)
    {
        if (!hasAccess(user, chan))
            goto NO_ACCESS;

        nick = skip((char *)text, ' ');
        irc_raw(bot, "MODE %s +o %s", chan, nick);
    }
    else if (strstr(text, "!deop") != NULL)
    {
        if (!hasAccess(user, chan))
            goto NO_ACCESS;

        nick = skip((char *)text, ' ');
        irc_raw(bot, "MODE %s -o %s", chan, nick);
    }
    else if (strstr(text, "!voice") != NULL)
    {
        if (!hasAccess(user, chan))
            goto NO_ACCESS;

        nick = skip((char *)text, ' ');
        irc_raw(bot, "MODE %s +v %s", chan, nick);
    }
    else if (strstr(text, "!devoice") != NULL)
    {
        if (!hasAccess(user, chan))
            goto NO_ACCESS;

        nick = skip((char *)text, ' ');
        irc_raw(bot, "MODE %s -v %s", chan, nick);
    }

    sn = 0;

NO_ACCESS:
    if (sn)
        irc_notice(bot, user, "You do not have access to use that command in %s", chan);
    free(buf);
}

MY_API void mod_init()
{
    register_module("chanop", "Aaron Blakely", "v0.1", "Channel Operator module");
    add_handler(PRIVMSG_CHAN, chanop_privmsg_handler);
}

MY_API void mod_unload()
{
    unregister_module("chanop");
    del_handler(PRIVMSG_CHAN, chanop_privmsg_handler);
}
