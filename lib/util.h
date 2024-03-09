/*
 * xbot: Just another IRC bot
 *
 * Written by Aaron Blakely <aaron@ephasic.org>
**/

#ifndef UTIL_H
#define UTIL_H

#include "logger.h"

#ifdef _WIN32
#define true TRUE
#define false FALSE
#endif

#ifdef _WIN32
#ifdef MY_DLL_EXPORTS
#define MY_API __declspec(dllexport)
#else
#define MY_API __declspec(dllimport)
#endif
#else
#define MY_API
#endif

void eprint(char *fmt, ...);

#if defined(__GLIBC__) && (__GLIBC__ < 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ < 38)) || defined(_WIN32)
MY_API void strlcpy(char *to, const char *from, int len);
#endif

#ifdef _WIN32
MY_API char *basename(char *path);
#endif

MY_API char *skip(char *s, char c);
MY_API void trim(char *s);

#endif
