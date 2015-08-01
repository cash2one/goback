local shaco = require "shaco"
local socket = require "socket"

local function exec(id, cmd)
    local f = io.popen(cmd, "r")
    local result = f:read("*l")
    while result do
        --result = util.iconv(result, "gbk", "utf-8")
        assert(socket.send(id, result.."\n"))
        result = f:read("*l")
    end
    assert(socket.send(id, "chicken node>\n"))
    f:close()
end

shaco.start(function()
    local host = shaco.getenv("host") or "127.0.0.1:7998"
    local ip, port = host:match("^([^:]+):?(%d+)$")
    local id = assert(socket.connect(ip, port))
    socket.readenable(id, true)
    assert(socket.send(id, "chicken"))
    local cmd = assert(socket.read(id, "*l"))
    assert(cmd == "ok")
    print ("login ok")
    while true do
        cmd = assert(socket.read(id, "*l"))
        exec(id, cmd)
    end
end)
