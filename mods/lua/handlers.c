#include "lua.h"
#include "events.h"

void lua_init_handlers()
{
    // see lib/events.h
    lua_setvar("PRIVMSG_CHAN", PRIVMSG_CHAN);
    lua_setvar("PRIVMSG_SELF", PRIVMSG_SELF);
    lua_setvar("TICK", TICK);
    lua_setvar("JOIN", JOIN);
    lua_setvar("JOIN_MYSELF", JOIN_MYSELF);
    lua_setvar("IRC_CONNECTED", IRC_CONNECTED);
    lua_setvar("NICK_MYSELF", NICK_MYSELF);
    lua_setvar("NICK_INUSE", NICK_INUSE);
    lua_setvar("CTCP", CTCP);
    lua_setvar("IRC_NAMREPLY", IRC_NAMREPLY);
    lua_setvar("IRC_WHOREPLY", IRC_WHOREPLY);
    lua_setvar("PART", PART);
    lua_setvar("PART_MYSELF", PART_MYSELF);
    lua_setvar("QUIT", QUIT);
    
    add_handler(PRIVMSG_CHAN, chanprivmsg_handler);
    add_handler(PRIVMSG_SELF, selfprivmsg_handler);
    add_handler(TICK, tick_handler);
    add_handler(JOIN, join_handler);
    add_handler(JOIN_MYSELF, joinmyself_handler);
    add_handler(IRC_CONNECTED, ircconnected_handler);
    add_handler(NICK_MYSELF, nickmyself_handler);
    add_handler(NICK_INUSE, nickinuse_handler);
    add_handler(CTCP, ctcp_handler);
    add_handler(IRC_NAMREPLY, ircnamreply_handler);
    add_handler(IRC_WHOREPLY, ircwhoreply_handler);
    add_handler(PART, part_handler);
    add_handler(PART_MYSELF, partmyself_handler);
    add_handler(QUIT, quit_handler);
}

void lua_unload_handlers()
{
    del_handler(PRIVMSG_CHAN, chanprivmsg_handler);
    del_handler(PRIVMSG_SELF, selfprivmsg_handler);
    del_handler(TICK, tick_handler);
    del_handler(JOIN, join_handler);
    del_handler(JOIN_MYSELF, joinmyself_handler);
    del_handler(IRC_CONNECTED, ircconnected_handler);
    del_handler(NICK_MYSELF, nickmyself_handler);
    del_handler(NICK_INUSE, nickinuse_handler);
    del_handler(CTCP, ctcp_handler);
    del_handler(IRC_NAMREPLY, ircnamreply_handler);
    del_handler(IRC_WHOREPLY, ircwhoreply_handler);
    del_handler(PART, part_handler);
    del_handler(PART_MYSELF, partmyself_handler);
    del_handler(QUIT, quit_handler);
}


MY_API void chanprivmsg_handler(struct irc_conn *bot, char *user, char *host, char *chan, const char *text)
{
    lua_fire_handlers(PRIVMSG_CHAN, user, host, chan, text);
    lua_eval(bot, user, host, chan, text);
    lua_load_script(bot, user, host, chan, text);
    lua_unload_script(bot, user, host, chan, text);
}

MY_API void selfprivmsg_handler(struct irc_conn *bot, char *user, char *host, const char *text)
{
    lua_fire_handlers(PRIVMSG_SELF, user, host, text);
}

MY_API void tick_handler(struct irc_conn *bot)
{
    lua_fire_handlers(TICK);
}

MY_API void join_handler(struct irc_conn *bot, char *user, char *host, char *chan)
{
    lua_fire_handlers(JOIN, user, host, chan);
}

MY_API void joinmyself_handler(struct irc_conn *bot, char *chan)
{
    lua_fire_handlers(JOIN_MYSELF, chan);
}

MY_API void ircconnected_handler(struct irc_conn *bot)
{
    lua_fire_handlers(IRC_CONNECTED);
}

MY_API void nickmyself_handler(struct irc_conn *bot, char *newnick)
{
    lua_fire_handlers(NICK_MYSELF, newnick);
}

MY_API void nickinuse_handler(struct irc_conn *bot, char *newnick)
{
    lua_fire_handlers(NICK_INUSE, newnick);
}

MY_API void ctcp_handler(struct irc_conn *bot, char *user, char *host, char *chan, const char *text)
{
    lua_fire_handlers(CTCP, user, host, chan, text);
}

MY_API void ircnamreply_handler(struct irc_conn *bot, char *chan, char *nicks)
{
    lua_fire_handlers(IRC_NAMREPLY, chan, nicks);
}

MY_API void ircwhoreply_handler(struct irc_conn *bot, char *chan, char *user, char *host, char *server, char *nick, char *flags, char *hops, char *realname)
{
    lua_fire_handlers(IRC_WHOREPLY, chan, user, host, server, nick, flags, hops, realname);
}

MY_API void part_handler(struct irc_conn *bot, char *user, char *host, char *chan)
{
    lua_fire_handlers(PART, user, host, chan);
}

MY_API void partmyself_handler(struct irc_conn *bot, char *chan)
{
    lua_fire_handlers(PART_MYSELF, chan);
}

MY_API void quit_handler(struct irc_conn *bot, char *user, char *host, const char *text)
{
    lua_fire_handlers(QUIT, user, host, text);
}
