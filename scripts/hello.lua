NAME = "hello"
VERSION = "v0.5"
AUTHOR = "Aaron Blakely"
DESCRIPTION = "A simple hello world script for xbot"

local handlers = {}

function hi(nick, host, chan, text)
    privmsg(chan, "Hello, " .. nick .. "!")
end

function load()
    -- register_script(NAME, VERSION, AUTHOR, DESCRIPTION)

    table.insert(handlers, {PRIVMSG_CHAN, add_handler(PRIVMSG_CHAN, hi)})
end

function unload()
    for i, v in ipairs(handlers) do
        del_handler(v[1], v[2])
    end
end
