/*
 * xbot: Just another IRC bot
 *
 * Written by Aaron Blakely <aaron@ephasic.org>
**/

#ifndef IRC_H
#define IRC_H

#include <stdio.h>

#ifdef _WIN32
#include <winsock2.h>
#endif

#define OUTBUF_SIZE 60000
#define INBUF_SIZE 60000

struct irc_conn
{
#ifdef _WIN32
	SOCKET srv_fd;
#else
	FILE *srv_fd;
#endif

    char nick[32];
    char admin[64];
    char host[256];
    char port[5];
    char real_name[512];

    // I/O Buffers
    char *out;
    char *in;
};

typedef struct handler event_handler;

#ifdef _WIN32
#ifdef MY_DLL_EXPORTS
#define MY_API __declspec(dllexport)
#else
#define MY_API __declspec(dllimport)
#endif
#else
#define MY_API
#endif

void irc_connect(struct irc_conn *bot);
void irc_auth(struct irc_conn *bot);

MY_API void irc_notice(struct irc_conn *bot, char *to, char *fmt, ...);
MY_API void irc_privmsg(struct irc_conn *bot, char *to, char *fmt, ...);
MY_API void irc_raw(struct irc_conn *bot, char *fmt, ...);
MY_API void irc_join(struct irc_conn *bot, char *channel);
MY_API void irc_part(struct irc_conn *bot, char *channel, char *reason);
MY_API void irc_ban(struct irc_conn *bot, char *channel, char *nick);
MY_API void irc_kick(struct irc_conn *bot, char *channel, char *user, char *reason);
MY_API void irc_mode(struct irc_conn *bot, char *channel, char *mode);
MY_API void irc_ctcp(struct irc_conn *bot, char *to, char *fmt, ...);

void irc_parse_raw(struct irc_conn *bot, char *raw);

#endif
