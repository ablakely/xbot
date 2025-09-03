#include "lua.h"
#include "channel.h"
#include "timers.h"
#include "module.h"
#include "logger.h"
#include <lua5.3/lauxlib.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <lua.h>
#else
#include <lua5.3/lua.h>
#endif

void lua_init_wrappers()
{
    lua_register(lua.L, "raw", raw_wrapper);
    lua_register(lua.L, "privmsg", privmsg_wrapper);
    lua_register(lua.L, "notice", notice_wrapper);
    lua_register(lua.L, "join", join_wrapper);
    lua_register(lua.L, "part", part_wrapper);
    lua_register(lua.L, "ban", ban_wrapper);
    lua_register(lua.L, "kick", kick_wrapper);
    lua_register(lua.L, "mode", mode_wrapper);
    lua_register(lua.L, "ctcp", ctcp_wrapper);
    lua_register(lua.L, "xlog", xlog_wrapper);

    lua_register(lua.L, "gethost", get_user_host_wrapper);
    lua_register(lua.L, "getuser", get_user_user_wrapper);
    lua_register(lua.L, "gethostmask", get_hostmask_wrapper);
    lua_register(lua.L, "chanexists", channel_exists_wrapper);
    lua_register(lua.L, "userexists", user_exists_wrapper);

    lua_register(lua.L, "isop", is_op_wrapper);
    lua_register(lua.L, "ishop", is_halfop_wrapper);
    lua_register(lua.L, "isvoice", is_voice_wrapper);
    lua_register(lua.L, "ison", is_on_channel_wrapper);
    lua_register(lua.L, "isbotadmin", is_botadmin_wrapper);

    lua_register(lua.L, "add_timer", add_timer_wrapper);
    lua_register(lua.L, "set_timer_name", set_timer_name_wrapper);
    lua_register(lua.L, "get_timer_repeat", get_timer_repeat_wrapper);
    lua_register(lua.L, "del_timer", del_timer_wrapper);
    lua_register(lua.L, "active_timers", active_timers_wrapper);

    lua_register(lua.L, "db_get", db_get);
    lua_register(lua.L, "db_set", db_set);
}

int xlog_wrapper(lua_State *L)
{
    char *msg = (char *)lua_tostring(L, 1);

    if (!msg)
        return 0;

    xlog("%s", msg);
    return 0;
}

int raw_wrapper(lua_State *L)
{
    char *text = (char *)lua_tostring(L, 1);

    if (!text)
        return 0;

    irc_raw(instance, text);
    return 0;
}

int privmsg_wrapper(lua_State *L)
{
    char *where = (char *)lua_tostring(L, 1);
    char *text = (char *)lua_tostring(L, 2);

    if (!where || !text)
        return 0;

    irc_privmsg(instance, where, text);
    return 0;
}

int notice_wrapper(lua_State *L)
{
    char *where = (char *)lua_tostring(L, 1);
    char *text = (char *)lua_tostring(L, 2);

    if (!where || !text)
        return 0;

    irc_notice(instance, where, text);
    return 0;
}

int join_wrapper(lua_State *L)
{
    char *chan = (char *)lua_tostring(L, 1);
    
    if (!chan)
        return 0;

    irc_join(instance, chan);
    return 0;
}

int part_wrapper(lua_State *L)
{
    char *chan = (char *)lua_tostring(L, 1);
    char *reason = (char *)lua_tostring(L, 2);

    if (!reason)
    {
        reason = "";
    }

    if (!chan)
        return 0;

    irc_part(instance, chan, reason);

    return 0;
}

int ban_wrapper(lua_State *L)
{
    char *chan = (char *)lua_tostring(L, 1);
    char *user = (char *)lua_tostring(L, 2);

    if (!chan || !user)
        return 0;

    irc_ban(instance, chan, user);
    return 0;
}

int kick_wrapper(lua_State *L)
{
    char *chan = (char *)lua_tostring(L, 1);
    char *user = (char *)lua_tostring(L, 2);
    char *reason = (char *)lua_tostring(L, 3);

    if (!reason)
    {
        reason = "";
    }

    if (!chan || !user)
        return 0;

    irc_kick(instance, chan, user, reason);

    return 0;
}

int mode_wrapper(lua_State *L)
{
    char *chan = (char *)lua_tostring(L, 1);
    char *mode = (char *)lua_tostring(L, 2);

    if (!chan || !mode)
        return 0;

    irc_mode(instance, chan, mode);
    return 0;
}

int ctcp_wrapper(lua_State *L)
{
    char *to = (char *)lua_tostring(L, 1);
    char *msg = (char *)lua_tostring(L, 2);

    if (!to || !msg)
        return 0;

    irc_ctcp(instance, to, msg);
    return 0;
}

// channel and user related functions

int get_user_host_wrapper(lua_State *L)
{
    char *host;
    char *user = (char *)lua_tostring(L, 1);

    if (!user)
        return 0;


    host = get_user_host(user);

    lua_pushstring(L, host);

    return 1;
}

int get_user_user_wrapper(lua_State *L)
{
    char *usr;
    char *user = (char *)lua_tostring(L, 1);

    if (!user)
        return 0;

    usr = get_user_user(user);

    lua_pushstring(L, usr);

    return 1;
}

int get_hostmask_wrapper(lua_State *L)
{
    char *hostmask;
    char *user = (char *)lua_tostring(L, 1);

    if (!user)
        return 0;

    hostmask = get_hostmask(user);
    lua_pushstring(L, hostmask);

    free(hostmask);
    return 1;
}

int channel_exists_wrapper(lua_State *L)
{
    int exists;
    char *chan = (char *)lua_tostring(L, 1);

    if (!chan)
        return 0;

    exists = channel_exists(chan);

    lua_pushboolean(L, exists);

    return 1;
}

int user_exists_wrapper(lua_State *L)
{
    int exists;
    char *user = (char *)lua_tostring(L, 1);

    if (!user)
        return 0;

    exists = user_exists(user);

    lua_pushboolean(L, exists);

    return 1;
}

int is_op_wrapper(lua_State *L)
{
    int isop;
    char *chan = (char *)lua_tostring(L, 1);
    char *user = (char *)lua_tostring(L, 2);

    if (!chan || !user)
        return 0;

    isop = is_op(chan, user);

    lua_pushboolean(L, isop);

    return 1;
}

int is_halfop_wrapper(lua_State *L)
{
    int ishop;
    char *chan = (char *)lua_tostring(L, 1);
    char *user = (char *)lua_tostring(L, 2);

    if (!chan || !user)
        return 0;

    ishop = is_halfop(chan, user);

    lua_pushboolean(L, ishop);

    return 1;
}

int is_voice_wrapper(lua_State *L)
{
    int isvoice;
    char *chan = (char *)lua_tostring(L, 1);
    char *user = (char *)lua_tostring(L, 2);
    
    if (!chan || !user)
        return 0;

    isvoice = is_voice(chan, user);

    lua_pushboolean(L, isvoice);

    return 1;
}

int is_on_channel_wrapper(lua_State *L)
{
    int ison;
    char *chan = (char *)lua_tostring(L, 1);
    char *user = (char *)lua_tostring(L, 2);

    if (!chan || !user)
        return 0;

    ison = is_on_channel(chan, user);

    lua_pushboolean(L, ison);

    return 1;
}

int is_botadmin_wrapper(lua_State *L)
{
    int isadmin;
    char *user = (char *)lua_tostring(L, 1);

    if (!user)
        return 0;

    isadmin = is_botadmin(user);

    lua_pushboolean(L, isadmin);

    return 1;
}

// timers

void timer_stub(struct irc_conn *bot, void *data)
{
    struct lua_timer *timer = (struct lua_timer *)data;

    lua_rawgeti(timer->L, LUA_REGISTRYINDEX, timer->lreg);

    if (timer->arg_ref != LUA_NOREF) {
        lua_rawgeti(timer->L, LUA_REGISTRYINDEX, timer->arg_ref);
        lua_call(timer->L, 1, 0);
    } else {
        lua_call(timer->L, 0, 0);
    }

}

int add_timer_wrapper(lua_State *L)
{
    int lreg, interval, repeat, arg_ref = LUA_NOREF;
    struct lua_timer *timer;

    if (lua_gettop(L) < 3)
    {
        xlog("[lua] Error: add_timer requires 3 arguments\n");
        return 0;
    }

    luaL_checktype(L, 1, LUA_TNUMBER);
    luaL_checktype(L, 2, LUA_TNUMBER);
    luaL_checktype(L, 3, LUA_TFUNCTION);

    interval = (int)lua_tointeger(L, 1);
    repeat   = (int)lua_tointeger(L, 2);

    lua_pushvalue(L, 3);
    lreg = luaL_ref(L, LUA_REGISTRYINDEX);

    if (lua_gettop(L) >= 4) {
        lua_pushvalue(L, 4);
        arg_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    }
    
    timer = calloc(1, sizeof(struct lua_timer));

    timer->repeat = repeat;
    timer->lreg = lreg;
    timer->arg_ref = arg_ref;
    timer->L = L;
    
    timer->timer_id = add_timer(get_bot(), interval, repeat, timer_stub, timer);

    lua_pushinteger(L, timer->timer_id);
    return 1;
}

int set_timer_name_wrapper(lua_State *L)
{
    int timerid;
    char *timername;

    if (lua_gettop(L) < 2) {
        xlog("[lua] Error: set_timer_name requires 2 arguments\n");
        return 0;
    }

    luaL_checktype(L, 1, LUA_TNUMBER);
    luaL_checktype(L, 2, LUA_TSTRING);

    timerid = (int)lua_tointeger(L, 1);
    timername = (char *)lua_tostring(L, 2);

    set_timer_name(timerid, timername);

    return 0;
}

int get_timer_repeat_wrapper(lua_State *L)
{
    int timerid, repeat;

    if (lua_gettop(L) < 1) {
        xlog("[lua] Error: get_timer_repeat requires 1 argument\n");
        return 0;
    }

    luaL_checktype(L, 1, LUA_TNUMBER);

    timerid = (int)lua_tointeger(L, 1);

    repeat = get_timer_repeat(timerid);

    lua_pushinteger(L, repeat);
    return 1;
}

int del_timer_wrapper(lua_State *L)
{
    int timerid;

    if (lua_gettop(L) < 1) {
        xlog("[lua] Error: del_timer requires 1 argument\n");
        return 0;
    }

    luaL_checktype(L, 1, LUA_TNUMBER);
    
    timerid = (int)lua_tointeger(L, 1);

    del_timer(timerid);
    return 0;
}

int active_timers_wrapper(lua_State *L)
{
    bool active = active_timers();

    lua_pushboolean(L, active);
    return 1;
}

// db 

int db_get(lua_State *L)
{
    char *dbkey;
    int dbtype;

    if (lua_gettop(L) < 1) {
        xlog("[lua] Error: db_get requires 1 argument\n");
        return 0;
    }

    luaL_checktype(L, 1, LUA_TSTRING);

    dbkey  = (char *)lua_tostring(L, 1);
    dbtype = db_get_hash_type(get_bot_db(), dbkey);

    switch (dbtype)
    {
        case DB_TYPE_CHAR:
            lua_pushstring(L, db_get_hash_char(get_bot_db(), dbkey));
            return 1;
        break;
        case DB_TYPE_INT:
            lua_pushnumber(L, db_get_hash_int(get_bot_db(), dbkey));
            return 1;
        break;
        case DB_TYPE_FLOAT:
            lua_pushnumber(L, db_get_hash_float(get_bot_db(), dbkey));
            return 1;
        break;
    }

    return 0;
}

int db_set(lua_State *L)
{
    return 0;
}
