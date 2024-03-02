#include "lua.h"

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
}

void raw_wrapper(lua_State *L)
{
    char *text = (char *)lua_tostring(L, 1);

    irc_raw(instance, text);
}

void privmsg_wrapper(lua_State *L)
{
    char *where = (char *)lua_tostring(L, 1);
    char *text = (char *)lua_tostring(L, 2);

    irc_privmsg(instance, where, text);
}

void notice_wrapper(lua_State *L)
{
    char *where = (char *)lua_tostring(L, 1);
    char *text = (char *)lua_tostring(L, 2);

    irc_notice(instance, where, text);
}

void join_wrapper(lua_State *L)
{
    char *chan = (char *)lua_tostring(L, 1);

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

    irc_part(instance, chan, reason);
}

void ban_wrapper(lua_State *L)
{
    char *chan = (char *)lua_tostring(L, 1);
    char *user = (char *)lua_tostring(L, 2);

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

    irc_kick(instance, chan, user, reason);
}

void mode_wrapper(lua_State *L)
{
    char *chan = (char *)lua_tostring(L, 1);
    char *mode = (char *)lua_tostring(L, 2);

    irc_mode(instance, chan, mode);
}

void ctcp_wrapper(lua_State *L)
{
    char *to = (char *)lua_tostring(L, 1);
    char *msg = (char *)lua_tostring(L, 2);
    va_list ap;

    irc_ctcp(instance, to, msg);
}
