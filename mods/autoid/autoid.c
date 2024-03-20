/*
 * autoid.c
 * Auto identify with nickserv
 *
 * Written by Aaron Blakely <aaron@ephasic.org>
 * Copyright 2024 (C) Aaron Blakely
*/

#include "util.h"
#define MY_DLL_EXPORTS 1

#include "module.h"
#include "irc.h"
#include "events.h"
#include "db.h"
#include "logger.h"
#include <string.h>
#include <stdlib.h>

struct irc_conn *instance;

MY_API void autoid_connected(struct irc_conn *bot, char *text)
{
    char *nickserv;
    char *password;

    nickserv = db_get_hash_char(get_bot_db(), "autoid.nickserv");
    password = db_get_hash_char(get_bot_db(), "autoid.password");

    if (nickserv && password)
    {
        xlog("[autoid] Identifying with %s\n", nickserv);
        irc_privmsg(bot, nickserv, "identify %s", password);
    }
}

MY_API void autoid_command(struct irc_conn *bot, char *user, char *host, const char *text)
{
    char *buf = strdup(text);
    char *args = skip(buf, ' ');
    char *arg1 = skip(args, ' ');
    char *arg2 = skip(arg1, ' ');
    char *arg3 = skip(arg2, ' ');
    char *nickserv;
    char *email;
    char *password;

    if (strncasecmp(text, "autoid", 6) == 0)
    {
        if (strncasecmp(args, "register", 8) == 0)
        {
            if (!arg1 || !arg2 || !arg3)
            {
                irc_notice(bot, user, "Usage: register <nickserv> <email> <password>");
                return;
            }

            if (!db_hash_exists(get_bot_db(), "autoid.password"))
            {
                irc_privmsg(bot, arg1, "register %s %s", arg3, arg2);
            }

            db_set_hash_char(get_bot_db(), "autoid.nickserv", arg1);
            db_set_hash_char(get_bot_db(), "autoid.email", arg2);
            db_set_hash_char(get_bot_db(), "autoid.password", arg3);
            db_write(get_bot_db(), bot->db_file);
        }
        else if (strncasecmp(args, "show", 4) == 0)
        {
            nickserv = db_get_hash_char(get_bot_db(), "autoid.nickserv");
            email = db_get_hash_char(get_bot_db(), "autoid.email");
            password = db_get_hash_char(get_bot_db(), "autoid.password");

            irc_notice(bot, user, "Nickserv: %s", nickserv);
            irc_notice(bot, user, "Email: %s", email);
            irc_notice(bot, user, "Password: %s", password);
        }
        else
        {
            irc_notice(bot, user, "Usage: autoid <register|show> [nickserv] [email] [password]");
        }
    }

    free(buf);
}

MY_API void mod_init()
{
    register_module("autoid", "Aaron Blakely", "v1.0", "Auto identify with nickserv");
    add_handler(IRC_CONNECTED, autoid_connected);
    add_handler(PRIVMSG_SELF, autoid_command);

    instance = get_bot();
}

MY_API void mod_unload()
{
    unregister_module("autoid");
    del_handler(IRC_CONNECTED, autoid_connected);
    del_handler(PRIVMSG_SELF, autoid_command);
}
