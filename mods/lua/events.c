#include "util.h"
#include "lua.h"
#include "events.h"
#include <string.h>
#include <stdlib.h>

int block = 0;

void lua_init_events()
{
    lua_register(lua.L, "_add_handler", lua_add_handler);
    lua_register(lua.L, "_del_handler", lua_del_handler);
}

int lua_add_handler(lua_State *L)
{
    char *event;
    int lreg;

    if (block)
    {
        // return error
        return !LUA_OK;
    }

    if (lua_gettop(L) < 2)
    {
        xlog("[lua] Error: add_handler requires 2 arguments\n");
        return 0;
    }

    luaL_checktype(L, 1, LUA_TSTRING);
    luaL_checktype(L, 2, LUA_TFUNCTION);

    lreg = luaL_ref(L, LUA_REGISTRYINDEX);
    if (lreg == -1)
    {
        const char *err = lua_tostring(L, -1);
        xlog("[lua/events/lua_add_handler] Error: %s\n", err);
    }

    event = (char *)lua_tostring(L, 1);

    xlog("[lua] Installing handler for event: %s : %d\n", event, lreg);

    strlcpy(lua.events[lua.event_count].event, event, 25);
    lua.events[lua.event_count].lreg = lreg;

    lua.event_count++;

    lua_pushinteger(L, lreg);
    return 1;
}

void lua_del_handler(lua_State *L)
{
    char *event;
    int i;
    int lreg;


    if (lua_gettop(L) < 2)
    {
        xlog("[lua] Error: del_handler requires 2 arguments\n");
        return;
    }

    luaL_checktype(L, 1, LUA_TSTRING);
    luaL_checktype(L, 2, LUA_TNUMBER);

    event = (char *)lua_tostring(L, 1);
    lreg = (int)lua_tointeger(L, 2);

    for (i = 0; i < lua.event_count; i++)
    {
        if (!strcmp(lua.events[i].event, event))
        {
            if (lua.events[i].lreg == lreg)
            {
                xlog("[lua] Removing handler for event: %s : %d\n", event, lreg);
                luaL_unref(L, LUA_REGISTRYINDEX, lua.events[i].lreg);

                while (i < lua.event_count)
                {
                    lua.events[i] = lua.events[i + 1];
                    i++;
                }

                lua.event_count--;
            }
        }
    }
}

void lua_callfunc(int lreg, int argc, ...)
{
    int i;
    va_list args;
    lua_State *L = lua.L;
    int stackSize = lua_gettop(L);

    va_start(args, argc);

    lua_rawgeti(L, LUA_REGISTRYINDEX, lreg);

    for (i = 0; i < argc; i++)
    {
        lua_pushstring(lua.L, va_arg(args, char *));
    }

    if (lua_pcall(lua.L, argc, 0, 0) != LUA_OK)
    {
        const char *err = lua_tostring(L, -1);
        xlog("[lua/events/lua_callfunc] Error: %s\n", err);
        lua_pop(L, 1);
    }

    lua_settop(L, stackSize);

    va_end(args);
}

void lua_fire_handlers(char *event, ...)
{
    int i;
    va_list args;
    char *user, *host, *chan, *text;

    for (i = 0; i < lua.event_count; i++)
    {
        if (!lua.events[i].lreg)
            continue;

        if (!strcmp(lua.events[i].event, event))
        {
            if (!strcmp(event, PRIVMSG_CHAN))
            {
                va_start(args, event);

                user = va_arg(args, char *);
                host = va_arg(args, char *);
                chan = va_arg(args, char *);
                text = va_arg(args, char *);

                lua_callfunc(lua.events[i].lreg, 4, user, host, chan, text);
            }
            else if (!strcmp(event, PRIVMSG_SELF))
            {
                va_start(args, event);

                user = va_arg(args, char *);
                host = va_arg(args, char *);
                text = va_arg(args, char *);

                lua_callfunc(lua.events[i].lreg, 3, user, host, text);
            }
            else if (!strcmp(event, JOIN))
            {
                va_start(args, event);

                user = va_arg(args, char *);
                host = va_arg(args, char *);
                chan = va_arg(args, char *);

                lua_callfunc(lua.events[i].lreg, 3, user, host, chan);
            }
            else if (!strcmp(event, JOIN_MYSELF))
            {
                va_start(args, event);

                chan = va_arg(args, char *);

                lua_callfunc(lua.events[i].lreg, 1, chan);
            }
            else if (!strcmp(event, PART))
            {
                va_start(args, event);

                user = va_arg(args, char *);
                host = va_arg(args, char *);
                chan = va_arg(args, char *);

                lua_callfunc(lua.events[i].lreg, 3, user, host, chan);
            }
            else if (!strcmp(event, PART_MYSELF))
            {
                va_start(args, event);

                chan = va_arg(args, char *);

                lua_callfunc(lua.events[i].lreg, 1, chan);
            }
            else if (!strcmp(event, QUIT))
            {
                va_start(args, event);

                user = va_arg(args, char *);
                host = va_arg(args, char *);
                text = va_arg(args, char *);

                lua_callfunc(lua.events[i].lreg, 3, user, host, text);
            }
            else if (!strcmp(event, NICK))
            {
                va_start(args, event);

                user = va_arg(args, char *);
                host = va_arg(args, char *);
                text = va_arg(args, char *);

                lua_callfunc(lua.events[i].lreg, 3, user, host, text);
            }
            else if (!strcmp(event, NICK_MYSELF))
            {
                va_start(args, event);

                text = va_arg(args, char *);

                lua_callfunc(lua.events[i].lreg, 1, text);
            }
            else if (!strcmp(event, CTCP))
            {
                va_start(args, event);

                user = va_arg(args, char *);
                host = va_arg(args, char *);
                text = va_arg(args, char *);

                lua_callfunc(lua.events[i].lreg, 3, user, host, text);
            }
            else if (!strcmp(event, IRC_CONNECTED))
            {
                lua_callfunc(lua.events[i].lreg, 0);
            }
            else if (!strcmp(event, IRC_NAMREPLY))
            {
                va_start(args, event);

                chan = va_arg(args, char *);
                text = va_arg(args, char *);

                lua_callfunc(lua.events[i].lreg, 2, chan, text);
            }
            else if (!strcmp(event, IRC_WHOREPLY))
            {
                va_start(args, event);

                chan = va_arg(args, char *);
                text = va_arg(args, char *);

                lua_callfunc(lua.events[i].lreg, 2, chan, text);
            }
            else if (!strcmp(event, NICK_INUSE))
            {
                lua_callfunc(lua.events[i].lreg, 0);
            }
            else if (!strcmp(event, TICK))
            {
                lua_callfunc(lua.events[i].lreg, 0);
            }
        }
    }

    va_end(args);
}
