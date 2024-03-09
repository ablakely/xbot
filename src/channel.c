#include "irc.h"
#include "util.h"
#include "events.h"
#include "module.h"
#include "channel.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

struct channel *channels[60];
int chan_count = 0;

void add_channel(char *name)
{
    if (channel_exists(name) == 1)
        return;

    xlog("[channel] Adding channel %s\n", name);
    channels[chan_count] = calloc(1, sizeof(struct channel));

    strlcpy(channels[chan_count]->name, name, 32);

    channels[chan_count]->user_count = 0;
    channels[chan_count]->users = calloc(256, sizeof(struct user));

    chan_count++;
}

void remove_channel(char *name)
{
    int i, j;

    for (i = 0; i < chan_count; i++)
    {
        if (!strcmp(channels[i]->name, name))
        {
            xlog("[channel] Removing channel %s\n", name);
            free(channels[i]->users);
            free(channels[i]);

            for (j = i; j < chan_count; j++)
            {
                channels[j] = channels[j + 1];
            }

            chan_count--;
        }
    }
}

struct user *get_user(char *nick)
{
    int i, j;

    for (i = 0; i < chan_count; i++)
    {
        for (j = 0; j < channels[i]->user_count; j++)
        {
            if (!strcmp(channels[i]->users[j].nick, nick))
            {
                return &channels[i]->users[j];
            }
        }
    }

    return NULL;
}

void add_user_to_channel(char *user, char *host, char *chan)
{
    int i;
    struct user *u, *uc;
    char buf[512];

#ifdef _WIN32
    BOOL is_op, is_voice, is_halfop, is_owner, is_admin;
#else
    bool is_op, is_voice, is_halfop, is_owner, is_admin;
#endif

    struct irc_conn *bot = get_bot();
    is_op = false;
    is_voice = false;
    is_halfop = false;
    is_owner = false;
    is_admin = false;


    if (!strcmp(chan, ""))
    {
        return;
    }

    // parse mode prefix symbols and remove them
    if (user[0] == '@')
    {
        is_op = true;
        user++;
    }
    else if (user[0] == '+')
    {
        is_voice = true;
        user++;
    }
    else if (user[0] == '%')
    {
        is_halfop = true;
        user++;
    }
    else if (user[0] == '~')
    {
        is_owner = true;
        user++;
    }
    else if (user[0] == '&')
    {
        is_admin = true;
        user++;
    }

    if (user_exists(chan, user) == 1)
        return;

    xlog("[channel] Adding user %s!%s to channel %s\n", user, host, chan);

    for (i = 0; i < chan_count; i++)
    {
        if (!strcmp(channels[i]->name, chan))
        {
            if (get_user(user) != NULL)
            {
                u = get_user(user);
                u->is_op = is_op | is_owner | is_admin;
                u->is_voice = is_voice | is_halfop | is_op | is_owner | is_admin;
                u->is_halfop = is_halfop;
                u->is_owner = is_owner;
                u->is_admin = is_admin;

                sprintf(buf, "%s!%s", u->nick, u->host);
                if (check_hostmask_match(bot->admin, buf) == true)
                {
                    u->is_botadmin = true;
                }

                channels[i]->users[channels[i]->user_count] = *u;
                channels[i]->user_count++;

                return;
            }

            strlcpy(channels[i]->users[channels[i]->user_count].nick, user, 50);
            strlcpy(channels[i]->users[channels[i]->user_count].host, host, 256);
            sprintf(buf, "%s!%s", user, host);

            channels[i]->users[channels[i]->user_count].is_op = is_op | is_owner | is_admin;
            channels[i]->users[channels[i]->user_count].is_voice = is_voice | is_halfop | is_op | is_owner | is_admin;
            channels[i]->users[channels[i]->user_count].is_halfop = is_halfop;
            channels[i]->users[channels[i]->user_count].is_owner = is_owner;
            channels[i]->users[channels[i]->user_count].is_admin = is_admin;

            if (check_hostmask_match(bot->admin, buf) == true)
            {
                channels[i]->users[channels[i]->user_count].is_botadmin = true;
            }

            channels[i]->user_count++;
        }
    }
}

void remove_user_from_channel(char *user, char *chan)
{
    int i, j;

    for (i = 0; i < chan_count; i++)
    {
        if (!strcmp(channels[i]->name, chan))
        {
            for (j = 0; j < channels[i]->user_count; j++)
            {
                if (!strcmp(channels[i]->users[j].nick, user))
                {
                    xlog("[channel] Removing user %s from channel %s\n", user, chan);

                    for (j = j; j < channels[i]->user_count; j++)
                    {
                        channels[i]->users[j] = channels[i]->users[j + 1];
                    }


                    channels[i]->user_count--;
                }
            }
        }
    }
}

void update_nick(char *old, char *new)
{
    int i, j;

    for (i = 0; i < chan_count; i++)
    {
        for (j = 0; j < channels[i]->user_count; j++)
        {
            if (!strcmp(channels[i]->users[j].nick, old))
            {
                strlcpy(channels[i]->users[j].nick, new, 50);
            }
        }
    }
}

void update_host(char *nick, char *host)
{
    int i, j;

    for (i = 0; i < chan_count; i++)
    {
        for (j = 0; j < channels[i]->user_count; j++)
        {
            if (!strcmp(channels[i]->users[j].nick, nick))
            {
                strlcpy(channels[i]->users[j].host, host, 256);
            }
        }
    }
}

void update_user(char *nick, char *user)
{
    int i, j;

    for (i = 0; i < chan_count; i++)
    {
        for (j = 0; j < channels[i]->user_count; j++)
        {
            if (!strcmp(channels[i]->users[j].nick, nick))
            {
                strlcpy(channels[i]->users[j].user, user, 50);
            }
        }
    }
}

void update_server(char *nick, char *server)
{
    int i, j;

    for (i = 0; i < chan_count; i++)
    {
        for (j = 0; j < channels[i]->user_count; j++)
        {
            if (!strcmp(channels[i]->users[j].nick, nick))
            {
                strlcpy(channels[i]->users[j].server, server, 256);
            }
        }
    }
}

void update_realname(char *nick, char *real_name)
{
    int i, j;

    for (i = 0; i < chan_count; i++)
    {
        for (j = 0; j < channels[i]->user_count; j++)
        {
            if (!strcmp(channels[i]->users[j].nick, nick))
            {
                strlcpy(channels[i]->users[j].real_name, real_name, 256);
            }
        }
    }
}

void user_quit(char *nick)
{
    int i, j;

    for (i = 0; i < chan_count; i++)
    {
        for (j = 0; j < channels[i]->user_count; j++)
        {
            if (!strcmp(channels[i]->users[j].nick, nick))
            {
                xlog("[channel] Removing user %s from channel %s\n", nick, channels[i]->name);

                for (j = j; j < channels[i]->user_count; j++)
                {
                    channels[i]->users[j] = channels[i]->users[j + 1];
                }

                channels[i]->user_count--;
            }
        }
    }
}

void set_realname(char *nick, char *real_name)
{
    int i, j;

    for (i = 0; i < chan_count; i++)
    {
        for (j = 0; j < channels[i]->user_count; j++)
        {
            if (!strcmp(channels[i]->users[j].nick, nick))
            {
                strlcpy(channels[i]->users[j].real_name, real_name, 256);
            }
        }
    }
}


MY_API char *get_user_host(char *nick)
{
    int i, j;

    for (i = 0; i < chan_count; i++)
    {
        for (j = 0; j < channels[i]->user_count; j++)
        {
            if (!strcmp(channels[i]->users[j].nick, nick))
            {
                return channels[i]->users[j].host;
            }
        }
    }

    return NULL;
}

char *get_user_user(char *nick)
{
    int i, j;

    for (i = 0; i < chan_count; i++)
    {
        for (j = 0; j < channels[i]->user_count; j++)
        {
            if (!strcmp(channels[i]->users[j].nick, nick))
            {
                return channels[i]->users[j].user;
            }
        }
    }

    return NULL;
}

#ifdef _WIN32
MY_API BOOL is_op(char *chan, char *nick)
#else
MY_API bool is_op(char *chan, char *nick)
#endif
{
    int i;

    for (i = 0; i < chan_count; i++)
    {
        if (!strcmp(channels[i]->name, chan))
        {
            int j;

            for (j = 0; j < channels[i]->user_count; j++)
            {
                if (!strcmp(channels[i]->users[j].nick, nick))
                {
                    return channels[i]->users[j].is_op;
                }
            }
        }
    }

    return 0;
}

#ifdef _WIN32
MY_API BOOL is_voice(char *chan, char *nick)
#else
MY_API bool is_voice(char *chan, char *nick)
#endif
{
    int i;

    for (i = 0; i < chan_count; i++)
    {
        if (!strcmp(channels[i]->name, chan))
        {
            int j;

            for (j = 0; j < channels[i]->user_count; j++)
            {
                if (!strcmp(channels[i]->users[j].nick, nick))
                {
                    return channels[i]->users[j].is_voice;
                }
            }
        }
    }

    return 0;
}

#ifdef _WIN32
MY_API BOOL is_halfop(char *chan, char *nick)
#else
MY_API bool is_halfop(char *chan, char *nick)
#endif
{
    int i;

    for (i = 0; i < chan_count; i++)
    {
        if (!strcmp(channels[i]->name, chan))
        {
            int j;

            for (j = 0; j < channels[i]->user_count; j++)
            {
                if (!strcmp(channels[i]->users[j].nick, nick))
                {
                    return channels[i]->users[j].is_halfop;
                }
            }
        }
    }

    return 0;
}

#ifdef _WIN32
MY_API BOOL is_owner(char *chan, char *nick)
#else
MY_API bool is_owner(char *chan, char *nick)
#endif
{
    int i;

    for (i = 0; i < chan_count; i++)
    {
        if (!strcmp(channels[i]->name, chan))
        {
            int j;

            for (j = 0; j < channels[i]->user_count; j++)
            {
                if (!strcmp(channels[i]->users[j].nick, nick))
                {
                    return channels[i]->users[j].is_owner;
                }
            }
        }
    }

    return 0;
}

#ifdef _WIN32
MY_API BOOL is_admin(char *chan, char *nick)
#else
MY_API bool is_admin(char *chan, char *nick)
#endif
{
    int i;

    for (i = 0; i < chan_count; i++)
    {
        if (!strcmp(channels[i]->name, chan))
        {
            int j;

            for (j = 0; j < channels[i]->user_count; j++)
            {
                if (!strcmp(channels[i]->users[j].nick, nick))
                {
                    return channels[i]->users[j].is_admin;
                }
            }
        }
    }

    return 0;
}

#ifdef _WIN32
MY_API BOOL channel_exists(char *chan)
#else
MY_API bool channel_exists(char *chan)
#endif
{
    int i;

    for (i = 0; i < chan_count; i++)
    {
        if (!strcmp(channels[i]->name, chan))
        {
            return 1;
        }
    }

    return 0;
}

#ifdef _WIN32
MY_API BOOL user_exists(char *chan, char *nick)
#else
MY_API bool user_exists(char *chan, char *nick)
#endif
{
    int i;

    for (i = 0; i < chan_count; i++)
    {
        if (!strcmp(channels[i]->name, chan))
        {
            int j;

            for (j = 0; j < channels[i]->user_count; j++)
            {
                if (!strcmp(channels[i]->users[j].nick, nick))
                {
                    return 1;
                }
            }
        }
    }

    return 0;
}

#ifdef _WIN32
MY_API BOOL is_on_channel(char *nick, char *chan)
#else 
MY_API bool is_on_channel(char *nick, char *chan)
#endif
{
    int i;

    for (i = 0; i < chan_count; i++)
    {
        if (!strcmp(channels[i]->name, chan))
        {
            int j;

            for (j = 0; j < channels[i]->user_count; j++)
            {
                if (!strcmp(channels[i]->users[j].nick, nick))
                {
                    return 1;
                }
            }
        }
    }

    return 0;
}

#ifdef _WIN32
MY_API BOOL is_botadmin(char *nick)
#else
MY_API bool is_botadmin(char *nick)
#endif
{
    int i, j;
    struct irc_conn *bot = get_bot();

    for (i = 0; i < chan_count; i++)
    {
        for (j = 0; j < channels[i]->user_count; j++)
        {
            if (!strcmp(channels[i]->users[j].nick, nick))
            {
                return channels[i]->users[j].is_botadmin;
            }
        }
    }

    return 0;
}
