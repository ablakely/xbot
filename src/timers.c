#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "timers.h"
#include "util.h"
#include "irc.h"

struct timers *timers;
int delete_queue[512];

void init_timers()
{
    timers = calloc(1, sizeof(struct timers));

    timers->count = 1;
    timers->timers = calloc(512, sizeof(struct timer));
}

MY_API int add_timer(struct irc_conn *bot, int interval, int repeat, void *handler, void *data)
{
    timers->timers[timers->count].interval = interval;
    timers->timers[timers->count].handler = handler;
    timers->timers[timers->count].id = timers->count;
    timers->timers[timers->count].bot = bot;
    timers->timers[timers->count].repeat = repeat;
    timers->timers[timers->count].repeat_count = 0;
    timers->timers[timers->count].data = data;
    timers->timers[timers->count].active = true;
    timers->timers[timers->count].next_run = time(NULL) + interval;
    timers->count++;

    return timers->count - 1;
}

MY_API void set_timer_name(int id, char *name)
{
    sprintf(timers->timers[id].name, "%s", name);
}

MY_API int get_timer_repeat(int id)
{
    return timers->timers[id].repeat_count;
}

#ifdef _WIN32
MY_API BOOL active_timers()
#else
MY_API bool active_timers()
#endif
{
    int i;

    for (i = 0; i < timers->count; i++)
    {
        if (timers->timers[i].active)
        {
            return true;
        }
    }

    return false;
}

void add_to_delete_queue(int id)
{
    int i;

    for (i = 0; i < 512; i++)
    {
        if (delete_queue[i] == 0)
        {
            delete_queue[i] = id;
            break;
        }
    }
}

MY_API void del_timer(int id)
{
    int i;

    for (i = 0; i < timers->count; i++)
    {
        if (timers->timers[i].id == id)
        {
            while (i < timers->count)
            {
                timers->timers[i] = timers->timers[i + 1];
                i++;
            }

            timers->count--;
        }
    }
}

void fire_timers()
{
    int i;
    void (*handler)();

    for (i = 0; i < timers->count; i++)
    {
        if (timers->timers[i].next_run <= time(NULL))
        {
            timers->timers[i].next_run = time(NULL) + timers->timers[i].interval;
            handler = timers->timers[i].handler;
            
            if (timers->timers[i].active)
                (*handler)(timers->timers[i].bot, timers->timers[i].data);

            if (timers->timers[i].repeat > 0)
            {
                timers->timers[i].repeat_count++;
                if (timers->timers[i].repeat_count >= timers->timers[i].repeat)
                {
                    timers->timers[i].active = false;
                    add_to_delete_queue(timers->timers[i].id);
                }
            }
        }
    }

    if (active_timers() == false)
    {
        for (i = 0; i < 512; i++)
        {
            if (delete_queue[i] != 0)
            {
                del_timer(delete_queue[i]);
                delete_queue[i] = 0;
            }
        }
    }
}
