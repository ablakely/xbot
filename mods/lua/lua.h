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
};

struct lua_interp
{
    lua_State *L;
    int count;
    struct lua_script *scripts;
};

extern struct lua_interp lua;

#endif
