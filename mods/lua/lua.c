#include "util.h"
#include "irc.h"
#include "events.h"
#include "module.h"
#include "channel.h"
#include "timers.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _WIN32
#define MY_DLL_EXPORTS 1
#include <libgen.h>
#endif

#include "lua.h"

struct lua_interp lua; 
struct irc_conn *instance;

int append_script(char *fname)
{
    char *scriptlist = db_get_hash_char(get_bot_db(), "lua.scripts");
    char *newlist = (char *)malloc(sizeof(char) * 500);
    char *p = scriptlist;

    if (scriptlist == NULL)
    {
        db_set_hash_char(get_bot_db(), "lua.scripts", fname);
        db_write(get_bot_db(), instance->db_file);
        return 0;
    }

    sprintf(newlist, "%s,%s", scriptlist, fname);
    printf("newlist: %s\n", newlist);

    db_set_hash_char(get_bot_db(), "lua.scripts", newlist);

    db_write(get_bot_db(), instance->db_file);

    free(newlist);

    return 0;
}

int remove_script(char *fname)
{
    char *scriptlist = db_get_hash_char(get_bot_db(), "lua.scripts");
    char *newlist = (char *)malloc(sizeof(char) * 500);
    char *p = scriptlist;
    char *q = newlist;
    int len = strlen(fname);
    int found = 0;

    if (scriptlist == NULL)
    {
        return 0;
    }

    while (*p != '\0')
    {
        if (strncmp(p, fname, len) == 0)
        {
            p = skip(p, ',');
            found = 1;
            continue;
        }

        *q = *p;
        p++;
        q++;
    }

    if (found == 0)
    {
        return 0;
    }

    *q = '\0';

    // remove trailing , if it exists

    if (newlist[strlen(newlist) - 1] == ',')
    {
        newlist[strlen(newlist) - 1] = '\0';
    }

    db_set_hash_char(get_bot_db(), "lua.scripts", newlist);
    db_write(get_bot_db(), instance->db_file);

    free(newlist);

    return 0;
}

struct script_list get_scripts()
{
    char *scriptlist = db_get_hash_char(get_bot_db(), "lua.scripts");
    struct script_list list = {0};
    char *p = scriptlist;
    int i = 0;

    if (scriptlist == NULL)
    {
        return list;
    }

    while (*p != '\0')
    {
        list.scripts[i] = p;
        p = skip(p, ',');
        i++;
    }

    list.count = i;

    return list;
}

void lua_eval(struct irc_conn *bot, char *user, char *host, char *chan, const char *text)
{
    int res;

    if (!is_botadmin(user))
        return;

    block = 1;
    if (strstr(text, "!lua") != NULL)
    {
        text = skip(text, ' ');
        res = luaL_loadstring(lua.L, text);

        lua_setvar("nick", user);
        lua_setvar("host", host);
        lua_setvar("chan", chan);

        if (res == LUA_OK)
        {
            res = lua_pcall(lua.L, 0, 0, 0);
            if (res != LUA_OK)
            {
                irc_privmsg(bot, chan, "Error: %s", lua_tostring(lua.L, -1));
                lua_pop(lua.L, 1);
            }
        }
        else
        {
            irc_privmsg(bot, chan, "Error: %s", lua_tostring(lua.L, -1));
            lua_pop(lua.L, 1);
        }
    }

    block = 0;
}

void lua_load_script(struct irc_conn *bot, char *user, char *host, char *chan, const char *text)
{
    char *name;
    char *script;
    char *buf;

    if (!is_botadmin(user))
        return;

    buf = (char *)malloc(sizeof(char *) * 500);

    if (strstr(text, "!load") != NULL)
    {
        text = skip((char *)text, ' ');
        sprintf(buf, "./scripts/%s", text);

        strlcpy(lua.scripts[lua.script_count].fname, buf, 150);

        if (luaL_loadfile(lua.L, buf) != LUA_OK)
        {
            if (!strcmp(chan, "-stdio-"))
            {
                xlog("[lua] Error loading lua script: %s\n", lua_tostring(lua.L, -1));
            } 
            else
            {
                irc_privmsg(bot, chan, "[lua] Error loading lua script: %s", lua_tostring(lua.L, -1));
                xlog("[lua] Error loading lua script: %s\n", lua_tostring(lua.L, -1));
            }

            return;
        }


        name = basename(buf);
        append_script(text);

        // execute the script
        if (lua_pcall(lua.L, 0, 0, 0) != LUA_OK)
        {
            if (!strcmp(chan, "-stdio-"))
            {
                xlog("[lua] Error executing lua script: %s\n", lua_tostring(lua.L, -1));
            }
            else
            {
                irc_privmsg(bot, chan, "Error executing lua script: %s", lua_tostring(lua.L, -1));
                xlog("[lua] Error executing lua script: %s\n", lua_tostring(lua.L, -1));
            }

            return;
        }

        // get a ref to unload function if it exists and store it with luaL_ref
        lua_getglobal(lua.L, "unload");

        if (lua_isfunction(lua.L, -1))
        {
            lua.scripts[lua.script_count].unload = luaL_ref(lua.L, LUA_REGISTRYINDEX);
        }
        else
        {
            lua.scripts[lua.script_count].unload = -1;

            if (!strcmp(chan, "-stdio-"))
            {
                xlog("[lua] No unload() function in %s\n", name);
            }
            else
            {
                irc_privmsg(bot, chan, "No unload() function in %s", name);
                xlog("[lua] No unload() function in %s [issued by %s!%s@%s]\n", name, user, host, chan);
            }
        }

        // call the load function if it exists
        lua_getglobal(lua.L, "load");

        if (lua_isfunction(lua.L, -1))
        {
            if (lua_pcall(lua.L, 0, 0, 0) != LUA_OK)
            {
                if (!strcmp(chan, "-stdio-"))
                {
                    xlog("[lua] Error calling load() in %s: %s\n", buf, lua_tostring(lua.L, -1));
                }
                else
                {
                    irc_privmsg(bot, chan, "Error calling load() in %s: %s", buf, lua_tostring(lua.L, -1));
                    xlog("[lua] Error calling load() in %s: %s\n", buf, lua_tostring(lua.L, -1));
                }

                return;
            }

            lua.script_count++;

            if (!strcmp(chan, "-stdio-"))
            {
                xlog("[lua] Loaded %s\n", name);
            }
            else
            {
                irc_privmsg(bot, chan, "Loaded %s", name);
                xlog("[lua] Loaded %s [issued by %s!%s@%s]\n", name, user, host, chan);
            }
        }
        else
        {
            if (!strcmp(chan, "-stdio-"))
            {
                xlog("[lua] Error: No load() function in %s\n", buf);
            }
            else
            {
                irc_privmsg(bot, chan, "Error: No load() function in %s", buf);
                xlog("[lua] Error: No load() function in %s [issued by %s!%s@%s]\n", buf, user, host, chan);
            }
        }
    }

    free(buf);
}

void lua_unload_script(struct irc_conn *bot, char *user, char *host, char *chan, const char *text)
{
    int i;
    char *name;
    char *script;
    char *buf;

    if (!is_botadmin(user))
        return;

    buf = (char *)malloc(sizeof(char *) * 500);
    
    if (strstr(text, "!unload") != NULL)
    {
        text = skip((char *)text, ' ');
        sprintf(buf, "./scripts/%s", text);

        irc_privmsg(bot, chan, "Unloading %s", buf);

        for (i = 0; i < lua.script_count; i++)
        {
            if (strcmp(lua.scripts[i].fname, buf) == 0)
            {
                // call from the ref in .unload
                lua_rawgeti(lua.L, LUA_REGISTRYINDEX, lua.scripts[i].unload);
                if (lua_pcall(lua.L, 0, 0, 0) != LUA_OK)
                {
                    irc_privmsg(bot, chan, "Error calling unload() in %s: %s", buf, lua_tostring(lua.L, -1));
                    xlog("[lua] Error calling unload() in %s: %s\n", buf, lua_tostring(lua.L, -1));

                    free(buf);
                    return;
                }

                luaL_unref(lua.L, LUA_REGISTRYINDEX, lua.scripts[i].unload);
                lua.scripts[i].unload = -1;

                remove_script(text);

                irc_privmsg(bot, chan, "Unloaded %s", text);
                xlog("[lua] Unloaded %s [issued by %s!%s@%s]\n", text, user, host, chan);

                while (i < lua.script_count)
                {
                    lua.scripts[i] = lua.scripts[i + 1];
                    i++;
                }

                lua.script_count--;

                free(buf);
                return;
            }
        }

        irc_privmsg(bot, chan, "Error: %s not loaded", text);
        xlog("[lua] Error: %s not loaded [issued by %s!%s@%s]\n", text, user, host, chan);
    }


    free(buf);
}

void lua_setvar(char *name, char *value)
{
    lua_pushstring(lua.L, value);
    lua_setglobal(lua.L, name);
}

void mod_init()
{
    int i;
    char *buf = (char *)malloc(sizeof(char *) * 500);
    struct script_list list = {0};

    instance = get_bot();
    list = get_scripts();

    lua.scripts = calloc(512, sizeof(struct lua_script));
    lua.events  = calloc(1024, sizeof(struct lua_event));

    lua.script_count = 0;
    lua.event_count = 0;

    lua.L = luaL_newstate();
    luaL_openlibs(lua.L);
    lua_init_wrappers();
    lua_init_events();

    lua_init_handlers();
    register_module("lua", "Aaron Blakely", "v0.1", "Lua module", MOD_FLAG_NONE);

    // load init.lua
    if (luaL_loadfile(lua.L, "./mods/lua/init.lua") != LUA_OK)
    {
        xlog("[lua] Error loading init.lua: %s\n", lua_tostring(lua.L, -1));

        free(buf);
        return;
    }

    if (lua_pcall(lua.L, 0, 0, 0) != LUA_OK)
    {
        xlog("[lua] Error executing init.lua: %s\n", lua_tostring(lua.L, -1));

        free(buf);
        return;
    }

    for (i = 0; i < list.count; i++)
    {
        xlog("[lua] Loading %s\n", list.scripts[i]);

        sprintf(buf, "./scripts/%s", list.scripts[i]);
        strlcpy(lua.scripts[lua.script_count].fname, buf, 150);

        if (luaL_loadfile(lua.L, buf) != LUA_OK)
        {
            xlog("[lua] Error loading lua script: %s\n", lua_tostring(lua.L, -1));
            continue;
        }

        // execute the script
        if (lua_pcall(lua.L, 0, 0, 0) != LUA_OK)
        {
            xlog("[lua] Error executing lua script: %s\n", lua_tostring(lua.L, -1));
            continue;
        }

        // get a ref to unload function if it exists and store it with luaL_ref
        lua_getglobal(lua.L, "unload");

        if (lua_isfunction(lua.L, -1))
        {
            lua.scripts[lua.script_count].unload = luaL_ref(lua.L, LUA_REGISTRYINDEX);
        }
        else
        {
            lua.scripts[lua.script_count].unload = -1;
            xlog("[lua] No unload() function in %s\n", list.scripts[i]);
        }

        // call the load function if it exists
        lua_getglobal(lua.L, "load");

        if (lua_isfunction(lua.L, -1))
        {
            if (lua_pcall(lua.L, 0, 0, 0) != LUA_OK)
            {
                xlog("[lua] Error calling load() in %s: %s\n", buf, lua_tostring(lua.L, -1));
                continue;
            }

            lua.script_count++;
            xlog("[lua] Loaded %s\n", list.scripts[i]);
        }
        else
        {
            xlog("[lua] Error: No load() function in %s\n", buf);
        }
    }

    free(buf);
}

void mod_unload()
{
    lua_close(lua.L);

    free(lua.scripts);
    free(lua.events);

    unregister_module("lua");
    del_handler(PRIVMSG_CHAN, lua_eval);
}
