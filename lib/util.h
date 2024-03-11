/*
 * xbot: Just another IRC bot
 *
 * Written by Aaron Blakely <aaron@ephasic.org>
**/

#ifndef UTIL_H
#define UTIL_H

#include "logger.h"

#define DEBUG 1

#ifdef DEBUG
#define BREAKPOINT() asm("int $3")
#else
#define BREAKPOINT()
#endif



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

// skip - skip over characters in a string
// s - the string to skip over
// c - the character to skip over
//
// returns a pointer to the first non-matching character
MY_API char *skip(char *s, char c);

// trim - remove leading and trailing whitespace from a string
MY_API void trim(char *s);

#endif
