#include "util.h"
#include "irc.h"
#include "events.h"
#include "module.h"
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

char *scriptsfile = "./mods/lua/scripts";

int append_script(char *fname)
{
    int i;
    FILE *fp;
    char *buf = (char *)malloc(sizeof(char *) * 500);

    // check if the file is already in the list
    struct script_list list = get_scripts();

    for (i = 0; i < list.count; i++)
    {
        if (strcmp(list.scripts[i], fname) == 0)
        {
            free(buf);
            return 0;
        }
    }

    if ((fp = fopen(scriptsfile, "a")) == NULL)
    {
        free(buf);
        return -1;
    }

    sprintf(buf, "%s\n", fname);
    fputs(buf, fp);
    fclose(fp);

    free(buf);
    return 0;
}

int remove_script(char *fname)
{
    FILE *fp, *tmp;
    char *buf = (char *)malloc(sizeof(char *) * 500);
    char *tmpfile = "./mods/lua/scripts.tmp";

    if ((fp = fopen(scriptsfile, "r")) == NULL)
    {
        free(buf);
        return -1;
    }

    tmp = fopen(tmpfile, "w");

    while (fgets(buf, 500, fp) != NULL)
    {
        if (strcmp(buf, fname) != 0)
        {
            fputs(buf, tmp);
        }
    }

    fclose(fp);
    fclose(tmp);

    remove(scriptsfile);
    rename(tmpfile, scriptsfile);

    free(buf);
    return 0;
}

struct script_list get_scripts()
{
    FILE *fp;
    char *buf = (char *)malloc(sizeof(char *) * 500);
    struct script_list list;
    int i = 0;

    if ((fp = fopen(scriptsfile, "r")) == NULL)
    {
        free(buf);

        list.count = 0;
        return list;
    }

    while (fgets(buf, 500, fp) != NULL)
    {
        list.scripts[i] = (char *)malloc(sizeof(char *) * 150);
        strlcpy(list.scripts[i], buf, 150);

        // remove newline
        list.scripts[i][strlen(list.scripts[i]) - 1] = '\0';


        i++;
    }

    list.count = i;

    free(buf);
    return list;
}

void lua_eval(struct irc_conn *bot, char *user, char *host, char *chan, const char *text)
{
    int res;

    block = 1;
    printf("lua eval called with %s\n", text);
    if (strstr(text, "!lua") != NULL)
    {
        text = skip(text, ' ');
        printf("lua: %s\n", text);

        res = luaL_loadstring(lua.L, text);

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
    char *buf = (char *)malloc(sizeof(char *) * 500);

    if (strstr(text, "!load") != NULL)
    {
        text = skip((char *)text, ' ');
        sprintf(buf, "./scripts/%s", text);

        strlcpy(lua.scripts[lua.script_count].fname, buf, 150);

        if (luaL_loadfile(lua.L, buf) != LUA_OK)
        {
            if (!strcmp(chan, "-stdio-"))
            {
                printf("Error loading lua script: %s\n", lua_tostring(lua.L, -1));
            } 
            else
            {
                irc_privmsg(bot, chan, "Error loading lua script: %s", lua_tostring(lua.L, -1));
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
                printf("Error executing lua script: %s\n", lua_tostring(lua.L, -1));
            }
            else
            {
                irc_privmsg(bot, chan, "Error executing lua script: %s", lua_tostring(lua.L, -1));
            }

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

            if (!strcmp(chan, "-stdio-"))
            {
                printf("No unload() function in %s\n", name);
            }
            else
            {
                irc_privmsg(bot, chan, "No unload() function in %s", name);
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
                    printf("Error calling load() in %s: %s\n", buf, lua_tostring(lua.L, -1));
                }
                else
                {
                    irc_privmsg(bot, chan, "Error calling load() in %s: %s", buf, lua_tostring(lua.L, -1));
                }

                return;
            }

            lua.script_count++;

            if (!strcmp(chan, "-stdio-"))
            {
                printf("Loaded %s\n", name);
            }
            else
            {
                irc_privmsg(bot, chan, "Loaded %s", name);
            }
        }
        else
        {
            if (!strcmp(chan, "-stdio-"))
            {
                printf("Error: No load() function in %s\n", buf);
            }
            else
            {
                irc_privmsg(bot, chan, "Error: No load() function in %s", buf);
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
    char *buf = (char *)malloc(sizeof(char *) * 500);

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
                    return;
                }

                luaL_unref(lua.L, LUA_REGISTRYINDEX, lua.scripts[i].unload);
                lua.scripts[i].unload = -1;

                remove_script(text);

                sprintf(buf, "Unloaded %s", text);
                irc_privmsg(bot, chan, buf);

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
    struct script_list list = get_scripts();
    instance = get_bot();

    lua.scripts = calloc(512, sizeof(struct lua_script));
    lua.events  = calloc(1024, sizeof(struct lua_event));

    lua.script_count = 0;
    lua.event_count = 0;

    lua.L = luaL_newstate();
    luaL_openlibs(lua.L);
    lua_init_wrappers();
    lua_init_events();

    lua_init_handlers();
    register_module("lua", "Aaron Blakely", "v0.1", "Lua module");

    // load init.lua
    if (luaL_loadfile(lua.L, "./mods/lua/init.lua") != LUA_OK)
    {
        printf("Error loading init.lua: %s\n", lua_tostring(lua.L, -1));
        return;
    }

    if (lua_pcall(lua.L, 0, 0, 0) != LUA_OK)
    {
        printf("Error executing init.lua: %s\n", lua_tostring(lua.L, -1));
        return;
    }

    for (i = 0; i < list.count; i++)
    {
        printf("Loading %s\n", list.scripts[i]);

        sprintf(buf, "!load %s", list.scripts[i]);

        lua_load_script(instance, "lua", "localhost", "-stdio-", buf);
    }

    printf("Lua module loaded\n");
    free(buf);
}

void mod_unload()
{
    lua_close(lua.L);

    free(lua.scripts);
    free(lua.events);

    unregister_module("lua");
    del_handler(PRIVMSG_CHAN, lua_eval);
    printf("Lua module unloaded\n");
}
