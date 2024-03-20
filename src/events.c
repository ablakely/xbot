#include "irc.h"
#include "util.h"
#include "events.h"
#include "channel.h"
#include "module.h"
#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#ifdef _WIN32
#include <ctype.h>
#endif

struct handler *handlers[512];
int handlers_count = 0;

void init_event_type(char *type)
{
    handlers[handlers_count] = calloc(1, sizeof(struct handler));
    handlers[handlers_count]->type = type;
    handlers[handlers_count]->count = 0;
    handlers[handlers_count]->evhands = calloc(128, sizeof(struct ev_handler));

    handlers_count++;
}

void init_events()
{
    init_event_type(TICK);
    init_event_type(RAW);
    init_event_type(PRIVMSG_SELF);
    init_event_type(PRIVMSG_CHAN);
    init_event_type(JOIN);
    init_event_type(JOIN_MYSELF);
    init_event_type(IRC_CONNECTED);
    init_event_type(NICK_MYSELF);
    init_event_type(NICK_INUSE);
    init_event_type(CTCP);
    init_event_type(IRC_NAMREPLY);
    init_event_type(IRC_WHOREPLY);
    init_event_type(MODE);
}

MY_API int add_handler(char *type, void *handler)
{
    int i;
    xlog("[events] Installing handler @ %p [type: %s]\n", handler, type);

    for (i = 0; i < handlers_count; i++)
    {
        if (!strcmp(handlers[i]->type, type))
        {
            if (handlers[i]->count < 128)
            {
                handlers[i]->evhands[handlers[i]->count].id = handlers[i]->count;
                handlers[i]->evhands[handlers[i]->count].handler = handler;

                handlers[i]->count++;

                return handlers[i]->count - 1;
            }
            else
            {
                xlog("[events] Handler array is full, cannot add more handlers.\n");
                return -1;
            }
        }
    }
}

MY_API void del_handler(char *type, void *handler)
{
    int i, j;
    for (i = 0; i < handlers_count; i++)
    {
        if (!strcmp(handlers[i]->type, type))
        {
            for (j = 0; j < handlers[i]->count; j++)
            {
                if (handlers[i]->evhands[j].handler == handler)
                {
                    handlers[i]->evhands[j].handler = NULL;
                }
            }
        }
    }
}

void fire_handler(struct irc_conn *bot, char *type, ...)
{
    va_list args;
    char *usr;
    char *host;
    char *chan;
    char *text;
    int i, j;
    void (*handler)();
    char linebuf[1024];
    char *cmd, *arg, *modpath;

    modpath = (char *)malloc(sizeof(char)*500);

    for (i = 0; i < handlers_count; i++)
    {
        if (!strcmp(handlers[i]->type, type))
        {
            for (j = 0; j < handlers[i]->count; j++)
            {
                handler = handlers[i]->evhands[j].handler;

                if (handler == NULL)
                    continue;

                if (!strcmp(type, PRIVMSG_SELF))
                {
                    va_start(args, type);
                 
                    usr = va_arg(args, char*);
                    host = va_arg(args, char*);
                    text = va_arg(args, char*);

                    (*handler)(bot, usr, host, text);

                    va_end(args);
                }
                else if (!strcmp(type, PRIVMSG_CHAN))
                {
                    va_start(args, type);

                    usr = va_arg(args, char*);
                    host = va_arg(args, char*);
                    chan = va_arg(args, char*);
                    text = va_arg(args, char*);

                    (*handler)(bot, usr, host, chan, text);
                    va_end(args);
                }
                else if (!strcmp(type, JOIN))
                {
                    va_start(args, type);

                    chan = va_arg(args, char*);
                    usr = va_arg(args, char*);
                    host = va_arg(args, char*);

                    (*handler)(bot, chan, usr, host);
                    va_end(args);
                }
                else if (!strcmp(type, IRC_CONNECTED)) 
                {
                    va_start(args, type);

                    text = va_arg(args, char*);

                    (*handler)(bot, text);
                    va_end(args);
                }
            }
        }
    }
    

    if (!strcmp(type, PRIVMSG_SELF))
    {
        va_start(args, type);
     
        usr = va_arg(args, char*);
        host = va_arg(args, char*);
        text = va_arg(args, char*);

        cmd = text;
        arg = skip(cmd, ' ');

        if (!strcmp("JOIN", cmd))
        {
            if (is_botadmin(usr) == true)
            {
                irc_raw(bot, "JOIN :%s", arg);
            }
            else
            {
                irc_notice(bot, usr, "You are unauthorized to use this command.");
            }
        }
        else if (!strcmp("PART", cmd))
        {
            if (is_botadmin(usr) == true)
            {
                irc_raw(bot, "PART %s :Admin made me leave.", arg);
            }
            else
            {
                irc_notice(bot, usr, "You are unauthorized to use this command.");
            }
        }
        else if (!strcmp("HANDLERS", cmd))
        {
            if (is_botadmin(usr) == true)
            {
                for (i = 0; i < handlers_count; i++)
                {
                    irc_notice(bot, usr, "Handler type: %s\n", handlers[i]->type);
                    for (j = 0; j < handlers[i]->count; j++)
                    {
                        irc_notice(bot, usr, "Handler %d: %p\n", j, handlers[i]->evhands[j].handler);
                    }
                }
            }
            else
            {
                irc_notice(bot, usr, "You are unauthorized to use this command.");
            }
        }
        else if (!strcmp("LOADMOD", cmd))
        {
            if (is_botadmin(usr) == true)
            {
#ifdef _WIN32
                irc_notice(bot, usr, "Loading module: mods/%s.dll", arg);
                sprintf(modpath, "./mods/%s.dll", arg);
#else
                irc_notice(bot, usr, "Loading module: mods/%s.so", arg);
                sprintf(modpath, "./mods/%s.so", arg);
#endif
                load_module(bot, usr, PRIVMSG_SELF, modpath);
            }
            else
            {
                irc_notice(bot, usr, "You are unauthorized to use this command.");
            }
        }
        else if (!strcmp("UNLOADMOD", cmd))
        {
            if (is_botadmin(usr) == true)
            {
#ifdef _WIN32
                irc_notice(bot, usr, "Unloading module: mods/%s.dll", arg);
                sprintf(modpath, "./mods/%s.dll", arg);
#else
                irc_notice(bot, usr, "Unloading module: mods/%s.so", arg);
                sprintf(modpath, "./mods/%s.so", arg);
#endif
                unload_module(bot, usr, modpath);
            }
            else
            {
                irc_notice(bot, usr, "You are unauthorized to use this command.");
            }
        }
        else if (!strcmp("MODLIST", cmd))
        {
            if (is_botadmin(usr) == true)
            {
                list_modules(bot, usr);
            }
            else
            {
                irc_notice(bot, usr, "You are unauthorized to use this command.");
            }
        }

        va_end(args);
    }
    
    free(modpath);
}

void free_events()
{
    //free(handlers);
}
