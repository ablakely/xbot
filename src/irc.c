/*
 * irc.c: IRC connection and parser
 * xbot: Just another IRC bot
 *
 * Written by Aaron Blakely <aaron@ephasic.org>
**/

#include "irc.h"
#include "util.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>

void irc_connect(struct irc_conn *bot)
{
	int srv_fd;
	struct addrinfo hints;
	struct addrinfo *res, *r;

	memset(&hints, 0, sizeof hints);
	hints.ai_family		= AF_UNSPEC;
	hints.ai_socktype	= SOCK_STREAM;

	if (getaddrinfo(bot->host, bot->port, &hints, &res) != 0)
	{
		eprint("Error: Cannot resolve hostname '%s':", bot->host);
	}

	for (r = res; r; r->ai_next)
	{
		if ((srv_fd = socket(r->ai_family, r->ai_socktype, r->ai_protocol)) == -1)
		{
			continue;
		}

		if (connect(srv_fd, r->ai_addr, r->ai_addrlen) == 0)
		{
			break;
		}

		close(srv_fd);
	}

	freeaddrinfo(res);
	if (!r)
	{
		eprint("Error: Cannot connect to host '%s'\n", bot->host);
	}

	bot->srv_fd = fdopen(srv_fd, "r+");
}

void irc_auth(struct irc_conn *bot)
{
	irc_raw(bot, "NICK %s", bot->nick);
	irc_raw(bot, "USER %s localhost %s :xbot (v0.1) - developed by @Dark_Aaron", bot->nick, bot->host);
	fflush(bot->srv_fd);
}

void irc_raw(struct irc_conn *bot, char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(bot->out, sizeof bot->out, fmt, ap);
	va_end(ap);


	fprintf(bot->srv_fd, "%s\r\n", bot->out);
}

void irc_parse_raw(struct irc_conn *bot, char *raw)
{
	char *user, *par, *text;
	user = bot->host;

	if (!raw || !*raw)
	{
		return;
	}

	if (raw[0] == ':')
	{
		user = raw + 1;
		raw  = skip(user, ' ');

		if (raw[0] == '\0')
		{
			return;
		}

		skip(user, '!');
	}

	skip(raw, '\r');
	par  = skip(raw, ' ');
	text = skip(par, ':');
	trim(par);

	if (!strcmp("PONG", raw))
	{
		return;
	}

	if (!strcmp("PRIVMSG", raw))
	{
		//handle_privmsg(user, par, text)
		if (!strcmp("Dark_Aaron", user))
		{
			irc_raw(bot, "JOIN #bots");
		}
	}
	else if (!strcmp("PING", raw))
	{
		irc_raw(bot, "PONG %s", text);
	}
	else
	{
		if (!strcmp("NICK", raw) && !strcmp(user, bot->nick))
		{
			strlcpy(bot->nick, text, sizeof bot->nick);
		}
	}
}