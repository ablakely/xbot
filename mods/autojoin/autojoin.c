#include "channel.h"
#define MY_DLL_EXPORTS 1

#include "module.h"
#include "irc.h"
#include "events.h"
#include "db.h"
#include "channel.h"
#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct irc_conn *instance;

MY_API void aj_connected(struct irc_conn *bot, char *text)
{
    char *chanlist;
    char *chan;
    int chancount;
    int i;

    if (!db_hash_exists(get_bot_db(), "autojoin.channels"))
    {
        return;
    }

    chan = calloc(60, sizeof(char));
    chanlist = db_get_hash_char(get_bot_db(), "autojoin.channels");
    chancount = db_get_hash_int(get_bot_db(), "autojoin.count");

    for (i = 0; i < chancount; i++)
    {
        chan = strsep(&chanlist, ",");

        printf("Autojoining %s\n", chan);
        irc_join(bot, chan);
    }

    //free(chan);
}

MY_API void aj_command(struct irc_conn *bot, char *user, char *host, char *text)
{
    int chancount;
    int i;
    char *chan;
    char *chanlist;
    char *buf;
    char *args = skip(text, ' ');
    char *arg1 = skip(args, ' ');

    if (!strcmp(text, "autojoin"))
    {

       if (!is_botadmin(user))
       {
           irc_notice(bot, user, "You are not authorized to use this command");

           return;
       }

       if (args == NULL)
       {
           irc_notice(bot, user, "Usage: autojoin <add|del|list> <channel>");

           return;
       }

       if (!strcmp(args, "add"))
       {
           if (arg1 == NULL)
           {
               irc_notice(bot, user, "Usage: autojoin add <channel>");

               return;
           }

           if (!db_hash_exists(get_bot_db(), "autojoin.channels"))
           {
               db_set_hash_char(get_bot_db(), "autojoin.channels", arg1);
               db_set_hash_int(get_bot_db(), "autojoin.count", 1);
               db_write(get_bot_db(), instance->db_file);

               irc_notice(bot, user, "Channel added to autojoin list");
           } else {
               chanlist = db_get_hash_char(get_bot_db(), "autojoin.channels");
               chancount = db_get_hash_int(get_bot_db(), "autojoin.count");

               strlcat(chanlist, ",", 1024);
               strlcat(chanlist, arg1, 1024);
               chancount++;

               buf = strdup(chanlist);

               db_set_hash_char(get_bot_db(), "autojoin.channels", buf);
               db_set_hash_int(get_bot_db(), "autojoin.count", chancount);
               db_write(get_bot_db(), instance->db_file);

               irc_notice(bot, user, "Channel added to autojoin list");
           }

           if (!channel_exists(arg1))
           {
               irc_join(bot, arg1);
           }
       }
       else if (!strcmp(args, "del"))
       {
           args = skip(text, ' ');
           if (args == NULL)
           {
               irc_notice(bot, user, "Usage: autojoin del <channel>");

               return;
           }

           chanlist = db_get_hash_char(get_bot_db(), "autojoin.channels");
           chancount = db_get_hash_int(get_bot_db(), "autojoin.count");

           if (chanlist == NULL || chancount <= 0)
           {
               irc_notice(bot, user, "No channels to delete");
               return;
           }
           else
           {
               buf = (char *)malloc(sizeof(char *)*1024);
               memset(buf, 0, 1024);

               while (chanlist != NULL)
               {
                   chan = strsep(&chanlist, ",");
                   if (chan == NULL)
                       break;

                   if (!strcmp(chan, arg1))
                   {
                       chancount--;
                       continue;
                   }

                   if (chanlist != NULL)
                   {
                       strlcat(buf, chan, 1024);
                       strlcat(buf, ",", 1024);
                   }
                   else
                   {
                       strlcat(buf, chan, 1024);
                   }
               }

               if (buf[strlen(buf)-1] == ',')
                   buf[strlen(buf)-1] = '\0';

               printf("New channel list: %s\n", buf);

               db_set_hash_char(get_bot_db(), "autojoin.channels", buf);
               db_set_hash_int(get_bot_db(), "autojoin.count", chancount);
               db_write(get_bot_db(), instance->db_file);

               irc_notice(bot, user, "Channel removed from autojoin list");
               free(buf);
           }

           if (channel_exists(arg1))
           {
               irc_part(bot, arg1, "Removing from autojoin list");
           }

           return;
       }
       else if (!strcmp(args, "list"))
       {
           chanlist = db_get_hash_char(get_bot_db(), "autojoin.channels");
           buf = (char *)malloc(sizeof(char *)*1024);

           strlcpy(buf, "Autojoin channels: ", 1024);
           strlcat(buf, chanlist, 1024);

           irc_notice(bot, user, buf);

           free(buf);
       }
       else
       {
           irc_notice(bot, user, "Usage: autojoin <add|del|list> <channel>");
       }

   }
}

MY_API void mod_init()
{
    register_module("autojoin", "Aaron Blakely", "v1.0", "Autojoin module");
    add_handler(IRC_CONNECTED, aj_connected);
    add_handler(PRIVMSG_SELF, aj_command);

    instance = get_bot();
}

MY_API void mod_unload()
{
    unregister_module("autojoin");
    del_handler(IRC_CONNECTED, aj_connected);
    del_handler(PRIVMSG_SELF, aj_command);
}
