#define MY_DLL_EXPORTS 1

#include "irc.h"
#include "events.h"
#include "module.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define BUFFER_SIZE 512
#include <windows.h>
DWORD startTick;

void parseUptime(const char *output, struct irc_conn *bot, const char *where) {
    const char *keyword = "Statistics since ";
    const char *uptime_start;
    int month, day, year, hour, minute;
    SYSTEMTIME uptime_systemtime, current_time;
    ULONGLONG uptime_ticks, current_ticks;
    ULONGLONG uptime_seconds;
    char buf[BUFFER_SIZE];
    int days, hours, minutes;

    OSVERSIONINFOEX osInfo;
    ZeroMemory(&osInfo, sizeof(OSVERSIONINFOEX));
    osInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

    uptime_start = strstr(output, keyword);
    if (uptime_start == NULL) {
        printf("Failed to parse uptime information.\n");
        return;
    }

    // Skip the "Statistics since " keyword
    uptime_start += strlen(keyword);

    // Parse the date and time
    if (sscanf(uptime_start, "%d/%d/%d %d:%d", &month, &day, &year, &hour, &minute) != 5) {
        printf("Failed to parse date and time.\n");
        return;
    }

    // Get the current time
    GetLocalTime(&current_time);

    
    // Calculate the uptime
    uptime_systemtime.wYear = year;
    uptime_systemtime.wMonth = month;
    uptime_systemtime.wDayOfWeek = 0;
    uptime_systemtime.wDay = day;
    uptime_systemtime.wHour = hour;
    uptime_systemtime.wMinute = minute;
    uptime_systemtime.wSecond = 0;
    uptime_systemtime.wMilliseconds = 0;


    SystemTimeToFileTime(&uptime_systemtime, (FILETIME *)&uptime_ticks);
    SystemTimeToFileTime(&current_time, (FILETIME *)&current_ticks);

    uptime_seconds = (current_ticks - uptime_ticks) / 10000000UL; // Convert to seconds

    uptime_seconds -= 1470;

    days = uptime_seconds / (24 * 3600);
    hours = (uptime_seconds % (24 * 3600)) / 3600;
    minutes = (uptime_seconds % 3600) / 60;

    if (GetVersionEx((OSVERSIONINFO*)&osInfo)) {
        if (osInfo.dwMajorVersion == 10 && osInfo.dwMinorVersion == 0) {
            sprintf(buf, "Windows 10");
        } else if (osInfo.dwMajorVersion == 6 && osInfo.dwMinorVersion == 3) {
            sprintf(buf, "Windows 8.1");
        } else if (osInfo.dwMajorVersion == 6 && osInfo.dwMinorVersion == 2) {
            sprintf(buf, "Windows 8");
        } else if (osInfo.dwMajorVersion == 6 && osInfo.dwMinorVersion == 1) {
            // detect Windows 7 or Windows Server 2008 R2
            if (osInfo.wProductType == VER_NT_WORKSTATION) {
                sprintf(buf, "Windows 7");
            } else {
                sprintf(buf, "Windows Server 2008 R2");
            }
        } else if (osInfo.dwMajorVersion == 6 && osInfo.dwMinorVersion == 0) {
            // detect Windows Vista or Windows Server 2008

            if (osInfo.wProductType == VER_NT_WORKSTATION) {
                sprintf(buf, "Windows Vista");
            } else {
                sprintf(buf, "Windows Server 2008");
            }
        } else if (osInfo.dwMajorVersion == 5 && osInfo.dwMinorVersion == 1) {
            // detect Windows XP or Windows Server 2003

            if (osInfo.wProductType == VER_NT_WORKSTATION) {
                sprintf(buf, "Windows XP");
            } else {
                sprintf(buf, "Windows Server 2003");
            }
        } else if (osInfo.dwMajorVersion == 5 && osInfo.dwMinorVersion == 0) {
            sprintf(buf, "Windows 2000");
        } else {
            sprintf(buf, "Windows");
        }
    }

    irc_privmsg(bot, where, "%s: %d days, %d hours, and %d minutes up\n", buf, days, hours, minutes);
}

char *executeCommand(const char *command)
{
    char buffer[BUFFER_SIZE];
    char *old_result = NULL;
    char *result = NULL;
    FILE *fp;

    fp = _popen(command, "r");
    if (fp == NULL) {
        printf("Failed to run command\n");
        exit(1);
    }

    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        if (result == NULL) {
            result = _strdup(buffer);
        } else {
            old_result = result;
            result = malloc(strlen(old_result) + strlen(buffer) + 1);
            strcpy(result, old_result);
            strcat(result, buffer);
            free(old_result);
        }
    }

    _pclose(fp);
    return result;
}
#endif

MY_API void up(struct irc_conn *bot, char *user, char *host, char *chan, char *text)
{
#ifdef _WIN32
    const char *command = "net statistics server";
    char *output = executeCommand(command);

    if (!strcmp(text, "!uptime"))
    {
        parseUptime(output, bot, chan);
        free(output);
    }
#else
	char buf[100];
	FILE* file;

	if (!strcmp(text, "!uptime"))
	{
		 file = popen("uptime", "r");
		 fgets(buf, 100, file);
		 pclose(file);

		 irc_privmsg(bot, chan, "%s", buf);
	}
#endif
}

MY_API void mod_init()
{
#ifdef _WIN32
    startTick = GetTickCount();
#endif

    register_module("uptime", "Aaron Blakely", "v0.1", "Uptime module");
    printf("installing up handler\n");
	add_handler(PRIVMSG_CHAN, up);
}

MY_API void mod_unload()
{
    unregister_module("uptime");
    printf("unloading up handler\n");
    del_handler(PRIVMSG_CHAN, up);
}
