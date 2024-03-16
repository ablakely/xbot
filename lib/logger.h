#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include "util.h"

struct logger
{
    FILE *log;
    char log_file[256];
};

extern struct logger logger;

void log_init(char *file);
void log_close();

MY_API void xlog(char *fmt, ...);

#endif
