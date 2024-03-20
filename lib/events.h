#ifndef EVENTS_H
#define EVENTS_H

#include "irc.h"

#define PRIVMSG_SELF    "CMSG"
#define PRIVMSG_CHAN    "PMSG"
#define JOIN            "JOIN"
#define JOIN_MYSELF     "JOIN_MYSELF"
#define PART            "PART"
#define PART_MYSELF     "PART_MYSELF"
#define QUIT            "QUIT"
#define NICK            "NICK"
#define NICK_MYSELF     "NICK_MYSELF"
#define NICK_INUSE      "433"
#define CTCP            "CTCP"
#define IRC_CONNECTED   "001"
#define IRC_MOTD        "372"
#define IRC_END_MOTD    "376"
#define IRC_NAMREPLY    "353"
#define IRC_WHOREPLY    "352"
#define TICK            "TICK"
#define MODE            "MODE"
#define MODE_PARSED     "MODE_PARSED"
#define RAW             "RAW"

struct ev_handler
{
    int id;
    void *handler;
};

struct handler
{
    char *type;
    int count;
    struct ev_handler *evhands;
};

extern struct handler *handlers[512];

void init_events();
MY_API int add_handler(char *type, void *handler);
MY_API void del_handler(char *type, void *handler);
void fire_handler(struct irc_conn *bot, char *type, ...);

#endif
