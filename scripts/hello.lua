NAME = "hello"
VERSION = "v0.5"
AUTHOR = "Aaron Blakely"
DESCRIPTION = "A simple hello world script for xbot"


function hi(nick, host, chan, text)
    irc_privmsg(chan, "Hello, " .. nick .. "!")
end

function load()
    add_lua_handler("PRIVMSG_CHAN", hi)
end

function unload()
    remove_lua_handler("PRIVMSG_CHAN", hi)
end
