NAME = "hello"
VERSION = "v0.5"
AUTHOR = "Aaron Blakely"
DESCRIPTION = "A simple hello world script for xbot"

function hi(nick, host, chan, text)
    privmsg(chan, "Hello, " .. nick .. " from hello.lua!!!!!")
end

function load()
    -- register_script(NAME, VERSION, AUTHOR, DESCRIPTION)

    add_handler(PRIVMSG_CHAN, hi)
end

function unload()
    -- unregister_script(NAME)

    del_handler(PRIVMSG_CHAN, hi)
end
