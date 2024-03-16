#include "lua.h"
#include "channel.h"
#include <stdlib.h>

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
}

void xlog_wrapper(lua_State *L)
{
    char *msg = (char *)lua_tostring(L, 1);

    if (!msg)
        return;

    xlog("%s", msg);
}

void raw_wrapper(lua_State *L)
{
    char *text = (char *)lua_tostring(L, 1);

    if (!text)
        return;

    irc_raw(instance, text);
}

void privmsg_wrapper(lua_State *L)
{
    char *where = (char *)lua_tostring(L, 1);
    char *text = (char *)lua_tostring(L, 2);

    if (!where || !text)
        return;

    irc_privmsg(instance, where, text);
}

void notice_wrapper(lua_State *L)
{
    char *where = (char *)lua_tostring(L, 1);
    char *text = (char *)lua_tostring(L, 2);

    if (!where || !text)
        return;

    irc_notice(instance, where, text);
}

void join_wrapper(lua_State *L)
{
    char *chan = (char *)lua_tostring(L, 1);
    
    if (!chan)
        return;

    irc_join(instance, chan);
}

void part_wrapper(lua_State *L)
{
    char *chan = (char *)lua_tostring(L, 1);
    char *reason = (char *)lua_tostring(L, 2);

    if (!reason)
    {
        reason = "";
    }

    if (!chan)
        return;

    irc_part(instance, chan, reason);
}

void ban_wrapper(lua_State *L)
{
    char *chan = (char *)lua_tostring(L, 1);
    char *user = (char *)lua_tostring(L, 2);

    if (!chan || !user)
        return;

    irc_ban(instance, chan, user);
}

void kick_wrapper(lua_State *L)
{
    char *chan = (char *)lua_tostring(L, 1);
    char *user = (char *)lua_tostring(L, 2);
    char *reason = (char *)lua_tostring(L, 3);

    if (!reason)
    {
        reason = "";
    }

    if (!chan || !user)
        return;

    irc_kick(instance, chan, user, reason);
}

void mode_wrapper(lua_State *L)
{
    char *chan = (char *)lua_tostring(L, 1);
    char *mode = (char *)lua_tostring(L, 2);

    if (!chan || !mode)
        return;

    irc_mode(instance, chan, mode);
}

void ctcp_wrapper(lua_State *L)
{
    char *to = (char *)lua_tostring(L, 1);
    char *msg = (char *)lua_tostring(L, 2);

    if (!to || !msg)
        return;

    irc_ctcp(instance, to, msg);
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
