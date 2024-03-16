#include "irc.h"
#include "util.h"
#include "events.h"
#include "module.h"
#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

struct mods *mods;
struct irc_conn *instance;
struct db_table *dbinstance;

void init_mods()
{
    mods = calloc(1, sizeof(struct mods));
    mods->count = 0;
    mods->modules = calloc(512, sizeof(struct module));
}

void load_module(struct irc_conn *bot, char *where, char *stype, char *file)
{
    
	char *error = (char *)malloc(sizeof(char *)*1024);
#ifdef _WIN32
    DWORD err;

    strlcpy(mods->modules[mods->count].fname, file, 256);
    mods->modules[mods->count].handle = LoadLibrary(file);
	if (mods->modules[mods->count].handle == NULL)
	{
        err = GetLastError();
		sprintf(error, "Error loading %s: %lu", file, err);

		if (strcmp("runtime", stype))
		{
			eprint("%s\n", error);
			return;
		}
        else if (strcmp(PRIVMSG_CHAN, stype))
        {
            irc_notice(bot, where, error);
        }
        else
        {
            irc_notice(bot, where, error);
        }

        return;
    }

	mods->modules[mods->count].init = GetProcAddress(mods->modules[mods->count].handle, "mod_init");
	if (mods->modules[mods->count].init  == NULL)
	{
		DWORD err = GetLastError();

		sprintf(error, "Error loading mod_init() pointer for %s: %lu", file, err);
		eprint("Error: %s\n", error);

        if (strcmp("runtime", stype))
        {
            return;
        }
        else if (strcmp(PRIVMSG_CHAN, stype))
        {
            irc_notice(bot, where, error);
        }
        else
        {
            irc_notice(bot, where, error);
        }

		return;
    }

	((void(*)(void))mods->modules[mods->count].init)();

    mods->modules[mods->count].unload = GetProcAddress(mods->modules[mods->count].handle, "mod_unload");
    if (mods->modules[mods->count].unload  == NULL)
    {
        DWORD err = GetLastError();

        sprintf(error, "Error loading mod_unload() pointer for %s: %lu", file, err);
        eprint("Error: %s\n", error);

        if (strcmp("runtime", stype))
        {
            return;
        }
        else if (strcmp(PRIVMSG_CHAN, stype))
        {
            irc_notice(bot, where, error);
        }
        else
        {
            irc_notice(bot, where, error);
        }

        return;
    }

	if (strcmp("runtime", stype))
    {
        irc_notice(bot, where, "Module '%s' loaded.", file);
    }

    xlog("[module] Module '%s' loaded.\n", file);
    free(error);
#else
    void (*mod_init)();

    strlcpy(mods->modules[mods->count].fname, file, 256);
    mods->modules[mods->count].handle = dlopen(file, RTLD_NOW | RTLD_GLOBAL | RTLD_NODELETE);
    if (!mods->modules[mods->count].handle)
    {
        sprintf(error, "[module] Error loading %s: %s", file, dlerror());

        if (strcmp("runtime", stype))
        {
            eprint("%s\n", error);
            return;
        }
        else if (strcmp(PRIVMSG_CHAN, stype))
        {
            irc_notice(bot, where, error);
        }
        else
        {
            irc_notice(bot, where, error);
        }

        xlog("[module] %s", error);
        return;
    }

    dlerror();

    *(void **)(&mods->modules[mods->count].init) = dlsym(mods->modules[mods->count].handle , "mod_init");

    if ((error = dlerror()) != NULL)
    {
        if (strcmp("runtime", stype))
        {
            eprint("Error: %s\n", error);
            return;
        }
        else if (strcmp(PRIVMSG_CHAN, stype))
        {
            irc_notice(bot, where, error);
        }
        else
        {
            irc_notice(bot, where, error);
        }

        xlog("[module] Error: %s", error);
    }

    (*mods->modules[mods->count].init)();

    *(void **)(&mods->modules[mods->count].unload) = dlsym(mods->modules[mods->count].handle , "mod_unload");
    if ((error = dlerror()) != NULL)
    {
        //sprintf(error, "Error: %s", error);
        eprint("Error: %s\n", error);

        if (strcmp("runtime", stype))
        {
            return;
        }
        else if (strcmp(PRIVMSG_CHAN, stype))
        {
            irc_notice(bot, where, error);
        }
        else
        {
            irc_notice(bot, where, error);
        }
    }

    if ((error = dlerror()) != NULL)
    {
        //sprintf(error, "Error: %s", error);
        eprint("Error: %s\n", error);

        if (strcmp("runtime", stype))
        {
            return;
        }
        else if (strcmp(PRIVMSG_CHAN, stype))
        {
            irc_notice(bot, where, error);
        }
        else
        {
            irc_notice(bot, where, error);
        }
    }


    if (strcmp("runtime", stype))
    {
        irc_notice(bot, where, "Module '%s' loaded.", file);
    }
    else
    {
        xlog("[module] Module '%s' loaded.\n", file);
    }
    free(error);
#endif

    mods->count++;
}

void unload_module(struct irc_conn *bot, char *where, char *file)
{
    int i;
    for (i = 0; i < mods->count; i++)
    {
        if (strcmp(mods->modules[i].fname, file) == 0)
        {
            if (mods->modules[i].flags & MOD_FLAG_NO_UNLOAD)
            {
                if (strcmp(PRIVMSG_CHAN, where))
                {
                    irc_notice(bot, where, "Module '%s' cannot be unloaded.", file);
                }
                else
                {
                    xlog("[module] Module '%s' cannot be unloaded.\n", file);
                }
                return;
            }


            (*mods->modules[i].unload)();
#ifdef _WIN32
            FreeLibrary(mods->modules[i].handle);
#else
            dlclose(mods->modules[i].handle);
#endif

            if (strcmp(PRIVMSG_CHAN, where))
            {
                irc_notice(bot, where, "Module '%s' unloaded.", file);
            }
            else
            {
                xlog("[module] Module '%s' unloaded.\n", file);
            }

            while (i < mods->count)
            {
                mods->modules[i] = mods->modules[i+1];
                i++;
            }

            mods->count--;

            return;
        }
    }
}

void list_modules(struct irc_conn *bot, char *where)
{
    int i;
    char *msg = malloc(512);
    char *tmp = malloc(512);

    for (i = 0; i < mods->count; i++)
    {
        sprintf(tmp, "%s (%s) by %s - %s", mods->modules[i].name, mods->modules[i].version, mods->modules[i].author, mods->modules[i].description);
        irc_notice(bot, where, tmp);
    }

    free(msg);
    free(tmp);
}

MY_API void register_module(char *name, char *author, char *version, char *description, ...)
{
    va_list args;

    if (mods->count >= 512)
    {
        eprint("Error: Too many modules loaded.\n");
        return;
    }

    strlcpy(mods->modules[mods->count].name, name, 25);
    strlcpy(mods->modules[mods->count].author, author, 50);
    strlcpy(mods->modules[mods->count].version, version, 10);
    strlcpy(mods->modules[mods->count].description, description, 256);

    va_start(args, description);

    mods->modules[mods->count].flags = va_arg(args, int);

    if (mods->modules[mods->count].flags & MOD_FLAG_NO_UNLOAD)
    {
        xlog("[module] Module '%s' cannot be unloaded.\n", name);
    }

    va_end(args);
}

MY_API void unregister_module(char *name)
{
    int i;
    for (i = 0; i < mods->count; i++)
    {
        if (strcmp(mods->modules[i].fname, name) == 0)
        {
            while (i < mods->count)
            {
                mods->modules[i] = mods->modules[i+1];
                i++;
            }
            return;
        }
    }
}

MY_API struct mods *get_mods()
{
    return mods;
}

MY_API struct irc_conn *get_bot()
{
    return instance;
}

void set_bot(struct irc_conn *b)
{
    instance = b;
}

void set_bot_db(struct db_table *db)
{
    dbinstance = db;
}

MY_API struct db_table *get_bot_db()
{
    return dbinstance;
}
