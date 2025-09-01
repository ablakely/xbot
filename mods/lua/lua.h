#define MY_DLL_EXPORTS 1

#ifndef LUA_H
#define LUA_H

#ifdef _WIN32
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#else
#include <lua5.3/lua.h>
#include <lua5.3/lauxlib.h>
#include <lua5.3/lualib.h>
#endif
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

struct script_list
{
    int count;
    char *scripts[512];
};

struct lua_timer
{
    lua_State *L;
    int timer_id;
    int repeat;
    
    int lreg;
    int arg_ref;
};

extern int block;

extern struct lua_interp lua;
extern struct irc_conn *instance;

// events.c
void lua_init_events();
int lua_add_handler(lua_State *L);
int lua_del_handler(lua_State *L);
void lua_fire_handlers(char *event, ...);
void lua_callfunc(lua_State *L, int lreg, int argc, ...);

// wrappers.c
void lua_init_wrappers();
int xlog_wrapper(lua_State *L);
int raw_wrapper(lua_State *L);
int privmsg_wrapper(lua_State *L);
int notice_wrapper(lua_State *L);
int join_wrapper(lua_State *L);
int part_wrapper(lua_State *L);
int ban_wrapper(lua_State *L);
int kick_wrapper(lua_State *L);
int mode_wrapper(lua_State *L);
int ctcp_wrapper(lua_State *L);

int get_user_host_wrapper(lua_State *L);
int get_user_user_wrapper(lua_State *L);
int get_hostmask_wrapper(lua_State *L);
int channel_exists_wrapper(lua_State *L);
int user_exists_wrapper(lua_State *L);
int is_op_wrapper(lua_State *L);
int is_halfop_wrapper(lua_State *L);
int is_voice_wrapper(lua_State *L);
int is_on_channel_wrapper(lua_State *L);
int is_botadmin_wrapper(lua_State *L);

void timer_stub(struct irc_conn *bot, void *data);
int add_timer_wrapper(lua_State *L);
int set_timer_name_wrapper(lua_State *L);
int get_timer_repeat_wrapper(lua_State *L);
int del_timer_wrapper(lua_State *L);
int active_timers_wrapper(lua_State *L);

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
int append_script(char *fname);
int remove_script(char *fname);
struct script_list get_scripts();
MY_API void mod_init();
MY_API void mod_unload();

MY_API void lua_eval(struct irc_conn *bot, char *user, char *host, char *chan, const char *text);
MY_API void lua_load_script(struct irc_conn *bot, char *user, char *host, char *chan, const char *text);
MY_API void lua_unload_script(struct irc_conn *bot, char *user, char *host, char *chan, const char *text);

#endif
