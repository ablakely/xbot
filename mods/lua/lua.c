#define MY_DLL_EXPORTS 1

#include "util.h"
#include "irc.h"
#include "events.h"
#include "module.h"
#include "timers.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>

#include "lua.h"

struct lua_interp lua; 
struct irc_conn *instance;


MY_API void lua_eval(struct irc_conn *bot, char *user, char *host, char *chan, const char *text)
{
    printf("lua eval called with %s\n", text);
    if (strstr(text, "!lua") != NULL)
    {
        // skip the command
        text = skip(text, ' ');
        printf("lua: %s\n", text);

        if (luaL_dostring(lua.L, text))
        {
            irc_privmsg(bot, chan, "Error: %s", lua_tostring(lua.L, -1));
        }
    }
}

MY_API void lua_load_script(struct irc_conn *bot, char *user, char *host, char *chan, const char *text)
{
    char *name;
    char *script;
    char *buf = (char *)malloc(sizeof(char *) * 500);

    if (strstr(text, "!load") != NULL)
    {
        text = skip((char *)text, ' ');
        sprintf(buf, "./scripts/%s", text);

        strlcpy(lua.scripts[lua.script_count].fname, buf, 150);

        if (luaL_loadfile(lua.L, buf) != LUA_OK)
        {
            irc_privmsg(bot, chan, "Error loading lua script: %s", lua_tostring(lua.L, -1));
            return;
        }

        name = basename(buf);


        // execute the script
        if (lua_pcall(lua.L, 0, 0, 0) != LUA_OK)
        {
            irc_privmsg(bot, chan, "Error executing lua script: %s", lua_tostring(lua.L, -1));
            return;
        }

        // get a ref to unload function if it exists and store it with luaL_ref
        lua_getglobal(lua.L, "unload");

        if (lua_isfunction(lua.L, -1))
        {
            lua.scripts[lua.script_count].unload = luaL_ref(lua.L, LUA_REGISTRYINDEX);
            printf("dbg: unload ref: %d\n", lua.scripts[lua.script_count].unload);
        }
        else
        {
            lua.scripts[lua.script_count].unload = -1;
            irc_privmsg(bot, chan, "No unload() function in %s", name);
        }

        // call the load function if it exists
        lua_getglobal(lua.L, "load");

        if (lua_isfunction(lua.L, -1))
        {
            if (lua_pcall(lua.L, 0, 0, 0) != LUA_OK)
            {
                irc_privmsg(bot, chan, "Error calling load() in %s: %s", buf, lua_tostring(lua.L, -1));
                return;
            }

            sprintf(buf, "Loaded %s", name);
            lua.script_count++;

            irc_privmsg(bot, chan, buf);
        }
        else
        {
            irc_privmsg(bot, chan, "Error: No load() function in %s", buf);
        }
    }

    free(buf);
}

MY_API void lua_unload_script(struct irc_conn *bot, char *user, char *host, char *chan, const char *text)
{
    char *name;
    char *script;
    char *buf = (char *)malloc(sizeof(char *) * 500);

    if (strstr(text, "!unload") != NULL)
    {
        text = skip((char *)text, ' ');
        sprintf(buf, "./scripts/%s", text);

        irc_privmsg(bot, chan, "Unloading %s", buf);

        for (int i = 0; i < lua.script_count; i++)
        {
            if (strcmp(lua.scripts[i].fname, buf) == 0)
            {
                // call from the ref in .unload
                lua_rawgeti(lua.L, LUA_REGISTRYINDEX, lua.scripts[i].unload);
                if (lua_pcall(lua.L, 0, 0, 0) != LUA_OK)
                {
                    irc_privmsg(bot, chan, "Error calling unload() in %s: %s", buf, lua_tostring(lua.L, -1));
                    return;
                }

                luaL_unref(lua.L, LUA_REGISTRYINDEX, lua.scripts[i].unload);
                lua.scripts[i].unload = -1;

                sprintf(buf, "Unloaded %s", text);
                irc_privmsg(bot, chan, buf);

                lua.script_count--;

                free(buf);
                return;
            }
        }

        irc_privmsg(bot, chan, "Error: %s not loaded", text);
    }


    free(buf);
}

void lua_setvar(char *name, char *value)
{
    lua_pushstring(lua.L, value);
    lua_setglobal(lua.L, name);
}

MY_API void mod_init()
{
    instance = get_bot();

    lua.scripts = calloc(512, sizeof(struct lua_script));
    lua.events  = calloc(1024, sizeof(struct lua_event));

    lua.script_count = 0;
    lua.event_count = 0;

    lua.L = luaL_newstate();
    luaL_openlibs(lua.L);
    lua_init_wrappers();
    lua_init_events();

    register_module("lua", "Aaron Blakely", "v0.1", "Lua module");
    lua_init_handlers();

    printf("Lua module loaded\n");
}

MY_API void mod_unload()
{
    lua_close(lua.L);

    unregister_module("lua");
    del_handler(PRIVMSG_CHAN, lua_eval);
    printf("Lua module unloaded\n");
}
