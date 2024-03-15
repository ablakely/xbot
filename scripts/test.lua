local NAME = "hello"
local VERSION = "v0.5"
local AUTHOR = "Aaron Blakely"
local DESCRIPTION = "A simple hello world script for xbot"

function test(nick, host, chan, text)
    -- check if text contains "hello"


    if string.find(text, "h") then
        privmsg(chan, "Hello, " .. nick .. " from test.lua")
    end
end

function load()
    -- register_script(NAME, VERSION, AUTHOR, DESCRIPTION)

    add_handler(PRIVMSG_CHAN, test)
end

function unload()
    -- unregister_script(NAME)

    del_handler(PRIVMSG_CHAN, test)
end
