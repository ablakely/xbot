#include "logger.h"
#include "util.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <stdlib.h>

struct logger logger;

void log_init(char *file)
{
    logger.log = fopen(file, "a");
    strlcpy(logger.log_file, file, sizeof logger.log_file);
}

void log_close()
{
    fclose(logger.log);
}

void xlog(char *fmt, ...)
{
    va_list args;
    time_t tv;

    char *buf = calloc(8128, sizeof(char));
    char *msg = calloc(4096, sizeof(char));
    char *tbuf = calloc(64, sizeof(char));

    va_start(args, fmt);
    vsnprintf(msg, 4096, fmt, args);

    time(&tv);

    strftime(tbuf, 64, "%Y-%m-%d %H:%M:%S", localtime(&tv));

    sprintf(buf, "[%s] %s", tbuf, msg);
    printf("%s", buf);

    // write to log file
    if (logger.log)
    {
        fprintf(logger.log, "%s", buf);
        fflush(logger.log);
    }

    va_end(args);

    free(buf);
    free(msg);
    free(tbuf);
}
