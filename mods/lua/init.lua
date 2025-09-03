-- init.lua: xbot runtime script
-- Do not modify this file unless you know what you are doing.
--
-- Written by Aaron Blakely

local handlerstore = {}
local chancommandstore = {}

function add_handler(type, func)
    local ret = _add_handler(type, func)

    local callfilename = debug.getinfo(2, "S").source

    if ret then
        table.insert(handlerstore, {type, func, ret, callfilename})
    end
end

function del_handler(type, func)
    local callfilename = debug.getinfo(2, "S").source

    -- loop through the handlerstore and remove the handler
    for i, v in ipairs(handlerstore) do

        print("called with type: " .. type .. "callfilename: " .. callfilename)
        print("dbug: type: " .. v[1] .. " func: " .. v[3] .. " callfilename: " .. v[4])
        if v[2] == func then
            print("dbug: ref found")
        end

        if v[1] == type and v[2] == func and v[4] == callfilename then
            _del_handler(type, v[3])
            table.remove(handlerstore, i)
            return
        end
    end

    xlog("[lua/init.lua] del_handler: Handler not found")
end

function chan_command_stub(nick, host, chan, text)
    local cmd, args = string.match(text, "^!(%S+)%s*(.*)")

    if cmd then
        if chancommandstore[cmd] then
            if not args then
                args = nil
            end

            chancommandstore[cmd][1](nick, host, chan, args)
        end
    end
end

function add_chan_command(cmd, func)
    local cfname = debug.getinfo(2, "S").source

    chancommandstore[cmd] = {func, cfname}
end

function del_chan_command(cmd)
    chancommandstore[cmd] = nil
end

function __luaenv_init()
    add_handler(PRIVMSG_CHAN, chan_command_stub)
end

-- run init routine
__luaenv_init()
