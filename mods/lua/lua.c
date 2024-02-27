#define MY_DLL_EXPORTS 1

#include "util.h"
#include "irc.h"
#include "events.h"
#include "module.h"
#include "timers.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
        sprintf(buf, "../scripts/%s", text);

        strlcpy(lua.scripts[lua.script_count].fname, buf, 150);

        if (luaL_loadfile(lua.L, buf))
        {
            irc_privmsg(bot, chan, "Error: %s", lua_tostring(lua.L, -1));
            return;
        }

        sprintf(buf, "Loaded %s", name);
        lua.script_count++;

        irc_privmsg(bot, chan, buf);
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
