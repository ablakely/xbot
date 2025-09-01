/*
 * xbot: Just another IRC bot
 *
 * Written by Aaron Blakely <aaron@ephasic.org>
**/

#include <stdio.h>
#include <errno.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "util.h"
#include "logger.h"

void eprint(char *fmt, ...)
{
    char msg[4096];
    char bufout[4096];
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(msg, sizeof msg, fmt, ap);
    va_end(ap);

    sprintf(bufout, "%s", msg);
    if (fmt[0] && fmt[strlen(fmt) - 1] == ':')
    {
        sprintf(bufout, "%s\n", strerror(errno));
    }

    fprintf(stderr, "%s", bufout);
    xlog("%s", bufout);
}


#if defined(__GLIBC__) && (__GLIBC__ < 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ < 38)) || defined(_WIN32)
void strlcpy(char *to, const char *from, int len)
{
    memccpy(to, from, '\0', len);
    to[len-1] = '\0';
}
#endif

#ifdef _WIN32
char *basename(char *path)
{
    char *p = strrchr(path, '\\');
    return p ? p + 1 : path;
}
#endif


char *skip(char *s, char c)
{
    while (*s != c && *s != '\0')
    {
        s++;
    }

    if (*s != '\0')
    {
        *s++ = '\0';
    }

    return s;
}

void split_arg(char *s, char **cmd, char **arg)
{
    *cmd = s;
    *arg = NULL;

    while (*s && *s != ' ') s++;
    if (*s)
    {
        *s = '\0';      // terminate first word
        s++;
        while (*s == ' ') s++;  // skip spaces
        *arg = s;      // remainder
    }
}


void trim(char *s)
{
    char *e;

    e = s + strlen(s) - 1;
    while (isspace(*e) && e > s)
    {
        e--;
    }

    *(e + 1) = '\0';
}
