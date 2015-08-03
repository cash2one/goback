local shaco = require "shaco"
local socket = require "socket"

local socket_error = setmetatable({}, 
    {__tostring = function() return "[socket error]" end})
local function socket_assert(r, err)
    if not r then
        error(socket_error)
    else
        return r
    end
end

local C = {}
local R = {}
local CN = false

local function read_result(id)
    local h = assert(socket.read(id, "*2"))
    local s = assert(socket.read(id, h))
    return s
end

local function host()
    local host = shaco.getenv("host") or "0.0.0.0:7998"
    local ip, port = host:match("^([^:]+):?(%d+)$")
    local lid = assert(socket.listen(ip, port))
    print("[start] ch listen on "..host)
    socket.start(lid, function(id)
        socket.readenable(lid, false)
        shaco.fork(function()
            local ok, err = pcall(function()
                CN = true
                socket.start(id)
                socket.readenable(id, true)
                print ("[login] chicken")
                local cmd, len, ret
                while true do
                    cmd = table.remove(C,1)
                    if cmd then
                        print ("[cmd send] "..cmd)
                        assert(socket.send(id, cmd..'\n'))
                        while true do
                            len = assert(socket.read(id, "*2"))
                            if len > 0 then
                                ret = assert(socket.read(id, len))
                                table.insert(R, ret)
                            else
                                table.insert(R, 0) -- end of package
                                break
                            end
                        end
                        print ("[ret read] ok")
                    else
                        shaco.sleep(1)
                    end
                end
            end)
            if not ok then
                socket.close(id)
                socket.readenable(lid, true)
                CN = false
                error(err)
            end
        end)
    end)
end

local function controller()
    local host = shaco.getenv("controller") or "127.0.0.1:7997"
    local ip, port = host:match("^([^:]+):?(%d+)$")
    local lid = assert(socket.listen(ip, port))
    print("[start] cc listen on "..host)
    socket.start(lid, function(id)
        socket.readenable(lid, false)
        shaco.fork(function()
            local ok, err = pcall(function()
                socket.start(id)
                socket.readenable(id, true)
                print ("[login] controller")
                local cmd, ret
                while true do
                    cmd = socket_assert((socket.read(id, "\r\n")))
                    print ("[cmd read] "..cmd, #cmd)
                    table.insert(C, cmd)
                    socket.readenable(id, false)
                    while CN do
                        local ret = table.remove(R,1)
                        if ret == 0 then -- end of package
                            break
                        elseif ret then
                            socket_assert((socket.send(id, ret..'\r\n')))
                        else 
                            shaco.sleep(1)
                        end
                    end
                    if not CN then
                        socket_assert(socket.send(id, "[X] chicken disconected\n"))
                    end
                    socket_assert(socket.send(id, "chicken node>\n"))
                    socket.readenable(id, true)
                    print ("[ret send] ok")
                end
            end)
            if not ok then
                socket.close(id)
                socket.readenable(lid, true)
                error(err)
            end
        end)
    end)
end

shaco.start(function()
    shaco.fork(host)
    shaco.fork(controller)
end)
