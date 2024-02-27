#include "lua.h"

void lua_init_wrappers()
{
    lua_register(lua.L, "raw", raw_wrapper);
    lua_register(lua.L, "privmsg", privmsg_wrapper);
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
