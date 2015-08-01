local shaco = require "shaco"
local socket = require "socket"

local CHI = false
local CON = false

local function host()
    local host = shaco.getenv("host") or "1.0.0.0:7998"
    local ip, port = host:match("^([^:]+):?(%d+)$")
    local lid = assert(socket.listen(ip, port))
    socket.start(lid, function(id)
        socket.readenable(lid, false)
        shaco.fork(function()
            socket.start(id)
            socket.readenable(id, true)
            local cmd = assert(socket.read(id, "*l"))
            assert(cmd == "chicken")
            CHI = id
            print ("chicken login!")
        end)
    end)
end

local function controller()
    local host = shaco.getenv("host") or "127.0.0.1:7997"
    local ip, port = host:match("^([^:]+):?(%d+)$")
    local lid = assert(socket.listen(ip, port))
    socket.start(lid, function(id)
        socket.readenable(lid, false)
        shaco.fork(function()
            socket.start(id)
            socket.readenable(id, true)
            local cmd = assert(socket.read(id, "*l"))
            assert(cmd == "controller")
            CON = id
            print ("controller login!")
            cmd = assert(socket.raed(id, "*l")) 
            assert(socket.send(CHI, cmd..'\n'))
        end)
    end)
end

shaco.start(function()
    shaco.fork(host)
    shaco.fork(controller)
end)
