/*
 * xbot: Just another IRC bot
 *
 * Written by Aaron Blakely <aaron@ephasic.org>
**/

#ifndef IRC_H
#define IRC_H

#define VERSION "0.1.0"

#include <stdio.h>

#include "util.h"
#include "db.h"

#define OUTBUF_SIZE 1200000
#define INBUF_SIZE 1200000

#include <openssl/ssl.h>
#include <openssl/err.h>

#ifdef _WIN32
#include <winsock2.h>
#else
#include <stdbool.h>
#endif


struct irc_conn
{
#ifdef _WIN32
	SOCKET srv_fd;
#else
	FILE *srv_fd;
#endif

    int ssl_fd;
    SSL *ssl;
    SSL_CTX *ctx;

    char nick[50];
    char user[50];
    char admin[256];
    char host[256];
    char port[5];
    char real_name[512];

#ifdef _WIN32
    BOOL use_ssl;
    BOOL verify_ssl;
#else
    bool use_ssl;
    bool verify_ssl;
#endif

    char db_file[256];
    char log_file[256];
    struct db_table *db;

    // I/O Buffers
    char *out;
    char *in;
};

typedef struct handler event_handler;

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

#ifdef _WIN32
MY_API BOOL check_hostmask_match(char *mask, char *host);
#else
MY_API bool check_hostmask_match(char *mask, char *host);
#endif


#endif
