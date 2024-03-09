-- init.lua: xbot runtime script
-- Do not modify this file unless you know what you are doing.
--
-- Written by Aaron Blakely

local handlerstore = {}

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
