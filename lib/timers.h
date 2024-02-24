#ifndef TIMERS_H
#define TIMERS_H

#include "irc.h"
#include "util.h"
#include <time.h>
#ifdef _WIN32
#include <windef.h>
#else
#include <stdbool.h>
#endif

struct timer
{
    int id;
    char name[50];
    int interval;
    int repeat;
    int repeat_count;

#ifdef _WIN32
    BOOL active;
#else
    bool active;
#endif

    time_t next_run;
    struct irc_conn *bot;

    void *handler;
    void *data;
};

struct timers
{
    int count;
    struct timer *timers;
};

extern struct timers *timers;;

void init_timers();

MY_API int add_timer(struct irc_conn *bot, int interval, int repeat, void *handler, void *data);
MY_API void set_timer_name(int id, char *name);
MY_API int get_timer_repeat(int id);
MY_API void del_timer(int id);

#ifdef _WIN32
MY_API BOOL active_timers();
#else
MY_API bool active_timers();
#endif

void add_to_delete_queue(int id);
void fire_timers();

#endif
