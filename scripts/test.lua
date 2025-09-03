local NAME = "tests"
local VERSION = "v0.5"
local AUTHOR = "Aaron Blakely"
local DESCRIPTION = "Testing code"

local timerid = 0

function timer_test_cb(data)
    privmsg(data.where, "timer stub called for " .. data.who .. " timer cycles ".. get_timer_repeat(timerid))
end

function cmd_timer(nick, host, chan, args)
    privmsg(chan, nick .. " created a timer")

    timerid = add_timer(10, 3, timer_test_cb, { who = nick, where = chan })
end

function cmd_dbget(nick, host, chan, args)
    privmsg(chan, "db_get: lua.scripts = ".. db_get("lua.scripts"))
end

function cmd_test(nick, host, chan, args)
    privmsg(chan, "cmd_test: ".. args)
end

function load()
    --if register_script(NAME, VERSION, AUTHOR, DESCRIPTION) then
        add_chan_command("timer", cmd_timer)
        add_chan_command("dbget", cmd_dbget)
        add_chan_command("test", cmd_test)
    --end
end

function unload()
    --if unregister_script(NAME) then
        del_chan_command("timer")
        del_chan_command("dbget")
        del_chan_command("test")
    --end
end
