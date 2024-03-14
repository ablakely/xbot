#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libconfig.h>
#include "irc.h"
#include "util.h"
#include "module.h"

struct irc_conn read_config(struct irc_conn bot, char *file)
{
    int count, n;
    config_t cfg, *cf;
    const config_setting_t *autoload;
    const char *base = (const char*)malloc(sizeof(char) * 1024);
    const char *mod  = NULL;
    int boolbase;

    bot.verify_ssl = 0;
    bot.use_ssl = 0;

    cf = &cfg;
    config_init(cf);

    if (!config_read_file(cf, file))
    {
        xlog("[xbot.cfg:%d] Configuration error: %s\n",
            config_error_line(cf),
            config_error_text(cf)
        );

        config_destroy(cf);
        exit(-1);
    }

    if (config_lookup_string(cf, "bot.nick", &base))
        strlcpy(bot.nick, base, sizeof bot.nick);

    if (config_lookup_string(cf, "bot.user", &base))
        strlcpy(bot.user, base, sizeof bot.user);

    if (config_lookup_string(cf, "bot.real", &base))
        strlcpy(bot.real_name, base, sizeof bot.real_name);

    if (config_lookup_string(cf, "server.host", &base))
        strlcpy(bot.host, base, sizeof bot.host);

    if (config_lookup_string(cf, "server.port", &base))
        strlcpy(bot.port, base, sizeof bot.port);

    if (config_lookup_string(cf, "bot.admin", &base))
        strlcpy(bot.admin, base, sizeof bot.admin);

    if (config_lookup_string(cf, "bot.db", &base))
        strlcpy(bot.db_file, base, sizeof bot.db_file);

    if (config_lookup_string(cf, "bot.log", &base))
        strlcpy(bot.log_file, base, sizeof bot.log_file);

    if (config_lookup_bool(cf, "server.ssl", &boolbase))
        bot.use_ssl = boolbase;

    if (config_lookup_bool(cf, "server.ssl_verify", &boolbase))
        bot.verify_ssl = boolbase;


    config_destroy(cf);

    return bot;
}

void run_autoload(struct irc_conn *bot)
{
    int count, n;
    config_t cfg, *cf;
    const config_setting_t *autoload;
    const char *base = (const char*)malloc(sizeof(char) * 1024);
    const char *mod  = NULL;
    char *modpath    = (char *)malloc(sizeof(char) * 500);

    cf = &cfg;
    config_init(cf);

    if (!config_read_file(cf, "xbot.cfg"))
    {
        xlog("[xbot.cfg:%d] Configuration error: %s\n",
            config_error_line(cf),
            config_error_text(cf)
        );

        config_destroy(cf);
        exit(-1);
    }

    if (bot->use_ssl)
    {
        if (config_lookup_string(cf, "server.ssl_module", &base))
        {
            strlcpy(bot->ssl_module, base, sizeof bot->ssl_module);

            // Load the SSL module
#ifdef _WIN32
            sprintf(modpath, "./mods/%s.dll", bot->ssl_module);
#else
            sprintf(modpath, "./mods/%s.so", bot->ssl_module);
#endif
            load_module(bot, "main", "runtime", modpath);
        }
    }

    autoload = config_lookup(cf, "mods.autoload");
    count    = config_setting_length(autoload);

    for (n = 0; n < count; n++)
    {
        mod = config_setting_get_string_elem(autoload, n);
#ifdef _WIN32
        sprintf(modpath, "./mods/%s.dll", mod);
#else
        sprintf(modpath, "./mods/%s.so", mod);
#endif
        load_module(bot, "main", "runtime", modpath);
    }

    config_destroy(cf);
    free(modpath);
}
