#ifndef MODULE_H
#define MODULE_H

#include "irc.h"
#include "events.h"

#ifdef _WIN32
#include <windows.h>
#endif

struct module {
    char name[25];
    char author[50];
    char version[10];
    char description[256];

    char fname[256];

#ifdef _WIN32
    HMODULE handle;
    FARPROC init;
    FARPROC unload;
#else
    void *handle;
    void (*init)();
    void (*unload)();
#endif
};

struct mods {
    int count;
    struct module *modules;
};

extern struct mods *mods;

void init_mods();
void load_module(struct irc_conn *bot, char *where, char *stype, char *file);
void unload_module(struct irc_conn *bot, char *where, char *file);
void list_modules(struct irc_conn *bot, char *where);
MY_API void register_module(char *name, char *author, char *version, char *description);
MY_API void unregister_module(char *name);
MY_API struct mods *get_mods();

#endif
