#ifndef CHANNEL_H
#define CHANNEL_H

#ifdef _WIN32
#include <windef.h>
#else
#include <stdbool.h>
#endif

#include "util.h"
#include "irc.h"
#include "events.h"

#define CHAN_PRIV_OP    "@"
#define CHAN_PRIV_HALF  "%"
#define CHAN_PRIV_VOICE "+"
#define CHAN_PRIV_OWNER "~"
#define CHAN_PRIV_ADMIN "&"

// debug 352: chan: #bbsn, user: ~shadow, host: services.ephasic.org, server: memphis.ephasic.org, nick: Shadow, flags: H*, realname: shadow

struct user
{
    char nick[50];
    char user[50];
    char host[256];
    char real_name[512];
    char server[256];

#ifdef _WIN32
    BOOL is_op;
    BOOL is_halfop;
    BOOL is_voice;
    BOOL is_owner;
    BOOL is_admin;

    BOOL is_botadmin;
#else
    bool is_op;
    bool is_halfop;
    bool is_voice;
    bool is_owner;
    bool is_admin;

    bool is_botadmin;
#endif
};

struct channel
{
    char name[32];
    int user_count;

    struct user *users;
};

extern struct channel *channels[60];

void add_channel(char *name);
void remove_channel(char *name);
void add_user_to_channel(char *user, char *host, char *chan);
void remove_user_from_channel(char *user, char *chan);
void update_nick(char *old_nick, char *new_nick);
void update_host(char *nick, char *host);
void update_user(char *nick, char *user);
void update_server(char *nick, char *server);
void update_realname(char *nick, char *real_name);
void user_quit(char *nick);
void set_realname(char *nick, char *real_name);
struct user *get_user(char *nick);

MY_API char *get_user_host(char *nick);
MY_API char *get_user_user(char *nick);

#ifdef _WIN32
MY_API BOOL is_op(char *chan, char *nick);
MY_API BOOL is_halfop(char *chan, char *nick);
MY_API BOOL is_voice(char *chan, char *nick);
MY_API BOOL channel_exists(char *chan);
MY_API BOOL user_exists(char *chan, char *nick);
MY_API BOOL is_on_channel(char *nick, char *chan);
MY_API BOOL is_botadmin(char *nick);
#else
MY_API bool is_op(char *chan, char *nick);
MY_API bool is_halfop(char *chan, char *nick);
MY_API bool is_voice(char *chan, char *nick);
MY_API bool is_owner(char *chan, char *nick);
MY_API bool is_admin(char *chan, char *nick);
MY_API bool channel_exists(char *chan);
MY_API bool user_exists(char *chan, char *nick);
MY_API bool is_on_channel(char *nick, char *chan);
MY_API bool is_botadmin(char *nick);
#endif

#endif
