#ifndef LUA_H
#define LUA_H

#include <lua5.3/lua.h>
#include <lua5.3/lauxlib.h>
#include <lua5.3/lualib.h>
#include "irc.h"

struct lua_script
{
    char name[25];
    char author[50];
    char version[10];
    char description[256];

    char fname[256];
    int unload;
};

struct lua_event
{
    char event[25];
    int lreg;
};

struct lua_interp
{
    int script_count;
    int event_count;

    lua_State *L;

    struct lua_script *scripts;
    struct lua_event *events;
};


extern struct lua_interp lua;
extern struct irc_conn *instance;

// events.c
void lua_init_events();
int lua_add_handler(lua_State *L);
void lua_del_handler(lua_State *L);
void lua_fire_handlers(char *event, ...);

// wrappers.c
void lua_init_wrappers();
void raw_wrapper(lua_State *L);
void privmsg_wrapper(lua_State *L);

// handlers.c
void lua_init_handlers();
void lua_unload_handlers();

MY_API void chanprivmsg_handler(struct irc_conn *bot, char *user, char *host, char *chan, const char *text);
MY_API void selfprivmsg_handler(struct irc_conn *bot, char *user, char *host, const char *text);
MY_API void tick_handler(struct irc_conn *bot);
MY_API void join_handler(struct irc_conn *bot, char *user, char *host, char *chan);
MY_API void joinmyself_handler(struct irc_conn *bot, char *chan);
MY_API void ircconnected_handler(struct irc_conn *bot);
MY_API void nickmyself_handler(struct irc_conn *bot, char *newnick);
MY_API void nickinuse_handler(struct irc_conn *bot, char *newnick);
MY_API void ctcp_handler(struct irc_conn *bot, char *user, char *host, char *chan, const char *text);
MY_API void ircnamreply_handler(struct irc_conn *bot, char *chan, char *nicks);
MY_API void ircwhoreply_handler(struct irc_conn *bot, char *chan, char *user, char *host, char *server, char *nick, char *flags, char *hops, char *realname);
MY_API void part_handler(struct irc_conn *bot, char *user, char *host, char *chan);
MY_API void partmyself_handler(struct irc_conn *bot, char *chan);
MY_API void quit_handler(struct irc_conn *bot, char *user, char *host, const char *text);

// lua.c
void lua_setvar(char *name, char *value);
MY_API void lua_eval(struct irc_conn *bot, char *user, char *host, char *chan, const char *text);
MY_API void lua_load_script(struct irc_conn *bot, char *user, char *host, char *chan, const char *text);
MY_API void lua_unload_script(struct irc_conn *bot, char *user, char *host, char *chan, const char *text);

#endif
