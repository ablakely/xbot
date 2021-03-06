#include "irc.h"
#include "events.h"
#include <stdio.h>
#include <stdlib.h>
#include <libconfig.h>

void aj(struct irc_conn *bot, char *text)
{

	// TODO: 
	// Config Parser API

	int count, n;
	config_t cfg, *cf;
	const config_setting_t *autojoin;
	const char *base;
	const char *chan = NULL;

	cf = &cfg;
	config_init(cf);

	if (!config_read_file(cf, "./xbot.cfg"))
	{
		printf("[xbot.cfg:%d] Configuration error: %s\n",
			config_error_line(cf),
			config_error_text(cf)
		);

		config_destroy(cf);
		return;
	}

	autojoin = config_lookup(cf, "mods.mod_autojoin");
	count    = config_setting_length(autojoin);

	for (n = 0; n < count; n++)
	{
		chan = config_setting_get_string_elem(autojoin, n);
		irc_raw(bot, "JOIN :%s", chan);
	}

	config_destroy(cf);
}

void mod_init()
{
	add_handler(IRC_CONNECTED, aj);
}