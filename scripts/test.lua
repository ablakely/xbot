local NAME = "hello"
local VERSION = "v0.5"
local AUTHOR = "Aaron Blakely"
local DESCRIPTION = "A simple hello world script for xbot"
local timerid = 0

function timer_test(data)
    privmsg(data.where, "timer stub called for " .. data.who .. " timer cycles ".. get_timer_repeat(timerid))
end

function test(nick, host, chan, text)
    if string.find(text, "!timer") then
        privmsg(chan, nick .. " created a timer")

        timerid = add_timer(10, 3, timer_test, { who = nick, where = chan })
    end
end

function load()
    --if register_script(NAME, VERSION, AUTHOR, DESCRIPTION) then
        add_handler(PRIVMSG_CHAN, test)
    --end
end

function unload()
    --if unregister_script(NAME) then
        del_handler(PRIVMSG_CHAN, test)
    --end
end
