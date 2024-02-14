#include "irc.h"
#include "util.h"
#include "events.h"
#include "module.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#ifdef _WIN32
#include <ctype.h>

#define SPF sprintf_s
#else
#define SPF sprintf
#endif

struct handler *privmsg_self;
struct handler *privmsg_chan;
struct handler *chan_join;
struct handler *irc_connected;

int handlers_count = 0;
struct handler *handlers[512];

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
    init_event_type(PRIVMSG_SELF);
    init_event_type(PRIVMSG_CHAN);
    init_event_type(JOIN);
    init_event_type(IRC_CONNECTED);
}

MY_API int add_handler(char *type, void *handler)
{
    int i;
    printf("Installing handler @ %p [type: %s]\n", handler, type);

    for (i = 0; i < handlers_count; i++)
    {
        printf("comparing %s to %s\n", handlers[i]->type, type);
        if (!strcmp(handlers[i]->type, type))
        {
            if (handlers[i]->count < 128)
            {
                handlers[i]->evhands[handlers[i]->count].id = handlers[i]->count;
                handlers[i]->evhands[handlers[i]->count].handler = handler;

                handlers[i]->count++;

                printf("type %s count: %d\n", type, handlers[i]->count);
                return handlers[i]->count - 1;
            }
            else
            {
                printf("Handler array is full, cannot add more handlers.\n");
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
    char *usr = calloc(1, 64);
    char *chan = calloc(1, 64);
    char *text = calloc(1, 512);
    int i, j;
    void (*handler)();
    char *cmd, *arg, *modpath;

    modpath = (char *)malloc(sizeof(char)*500);

    printf("Firing handler for type: %s\n", type);

    if (!strcmp(type, PRIVMSG_SELF))
    {
        printf("Firing PRIVMSG_SELF handler\n");
        va_start(args, type);
     
        usr = va_arg(args, char*);
        text = va_arg(args, char*);

        cmd = text;
        arg = skip(cmd, ' ');

        printf("cmd: %s\n", cmd);
        printf("arg: %s\n", arg);

        if (!strcmp("JOIN", cmd))
        {
            printf("dbug: cmp (%s :  %s)\n", (char*)bot->admin, usr);

            if (!strcmp(bot->admin, usr))
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
            if (!strcmp(bot->admin, usr))
            {
                irc_raw(bot, "PART %s :Admin made me leave.", arg);
            }
            else
            {
                irc_notice(bot, usr, "You are unauthorized to use this command.");
            }
        }
        /*
        else if (!strcmp("PRINT_HANDLERS", cmd))
        {
            if (!strcmp(bot->admin, usr))
            {
                for (i = 0; i < privmsg_chan->count; i++)
                {
                    irc_notice(bot, usr, "handler[%i:%s]: %p", i, privmsg_chan->type, privmsg_chan->handlers[i]);
                }

                for (i = 0; i < privmsg_self->count; i++)
                {
                    irc_notice(bot, usr, "handler[%i:%s]: %p", i, privmsg_self->type, privmsg_self->handlers[i]);
                }

                for (i = 0; i < irc_connected->count; i++)
                {
                    irc_notice(bot, usr, "handler[%i:%s]: %p", i , irc_connected->type, irc_connected->handlers[i]);
                }
            }
        }
        */
        else if (!strcmp("LOADMOD", cmd))
        {
            if (!strcmp(bot->admin, usr))
            {
                irc_notice(bot, usr, "Loading module: mods/%s.so", arg);
#ifdef _WIN32
                SPF(modpath, "./mods/%s.dll", arg);
#else
                SPF(modpath, "./mods/%s.so", arg);
#endif
                load_module(bot, usr, PRIVMSG_SELF, modpath);
            }
            else
            {
                irc_notice(bot, usr, "You are unauthorized to use this command.");
            }
        }
    }
    
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
                    text = va_arg(args, char*);

                    (*handler)(bot, usr, text);
                    va_end(args);
                }
                else if (!strcmp(type, PRIVMSG_CHAN))
                {
                    va_start(args, type);

                    usr = va_arg(args, char*);
                    chan = va_arg(args, char*);
                    text = va_arg(args, char*);

                    (*handler)(bot, usr, chan, text);
                    va_end(args);
                }
                else if (!strcmp(type, JOIN))
                {
                    va_start(args, type);

                    chan = va_arg(args, char*);
                    usr = va_arg(args, char*);

                    (*handler)(bot, chan, usr);

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
    
    free(modpath);
}

void free_events()
{
    //free(handlers);
}
