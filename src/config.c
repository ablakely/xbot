#include <stdio.h>
#include <stdlib.h>
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
    char *modpath    = (char *)malloc(sizeof(char) * 500);


    cf = &cfg;
    config_init(cf);

    if (!config_read_file(cf, file))
    {
        printf("[xbot.cfg:%d] Configuration error: %s\n",
            config_error_line(cf),
            config_error_text(cf)
        );

        config_destroy(cf);
        exit(-1);
    }

    if (config_lookup_string(cf, "bot.nick", &base))
        strlcpy(bot.nick, base, sizeof bot.nick);

    if (config_lookup_string(cf, "server.host", &base))
        strlcpy(bot.host, base, sizeof bot.host);

    if (config_lookup_string(cf, "server.port", &base))
        strlcpy(bot.port, base, sizeof bot.port);

    if (config_lookup_string(cf, "bot.admin", &base))
        strlcpy(bot.admin, base, sizeof bot.admin);

    if (config_lookup_string(cf, "bot.db", &base))
        strlcpy(bot.db_file, base, sizeof bot.db_file);

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
        load_module(&bot, "main", "runtime", modpath);
    }

    config_destroy(cf);

    return bot;
}
