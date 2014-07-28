local reloading = { }

local SERVICE_NAME = "FILE_RELOADING_SERVICE"
local PORT = 22222
local QUERY_INTERVAL = 1000
local CONNECTION_TIMEOUT = 10000


local function onAsyncConnect(socket, connected)
	if connected then 
		local sock = TcpFromSocket(socket, 1024, 128)
		sock:blocking(false)
		reloading.connection = sock
		sock.outStream:writeString("tower_defence")
		sock.outStream:flush()
		Log.info("Connected to reloading!")
	else 
		C.serviceFinderAsync(SERVICE_NAME, PORT, onReload, QUERY_INTERVAL)
	end
end

local function onReload(event, success)
	if success then 
		local buffer = event.buffer
		local ip 	= C.bufferReadInt(buffer)
		local port 	= C.bufferReadShort(buffer)
		local sock  = C.socketCreate(C.TCP_SOCKET)

		C.socketAsyncConnect(sock, ip, port, CONNECTION_TIMEOUT, onAsyncConnect)
	else
		C.serviceFinderAsync(SERVICE_NAME, PORT, onReload, QUERY_INTERVAL)
	end
end

local function reloadItem()
	local sock = reloading.connection
	local stream = sock.inStream

	sock:receiveTimeout(1000)
	sock:blocking(true)
	Log.infof("Data length: %s", stream:dataLength())

	local numFiles = stream:readShort()
	Log.infof("Received some files to reload %d", numFiles)

	local mainFile = nil

	for i=1, numFiles, 1 do
		local name = stream:readString()
		if i == 1 then 
			mainFile = name
		end 
		
		Log.infof("File name: %s", name)
		local result = C.receiveFile(stream.cHandle, Game.resourceDir, name)
	end

	sock:blocking(false)

	local name, ext = Path.baseName(mainFile)
	if ext ~= "lua" then
		reloading.manager:reloadFromName(mainFile)
	else 
		local reloadPath = Game.name .. "/" .. mainFile

		local reloadName = nil
		for k, v in pairs(R) do
			if v.path == reloadPath then 
				reloadName = Game.name .. "/" .. k
				break
			end
		end
		
		runExternalFile(reloadPath, reloadName)
	end 
end

function global.initializeReloading(manager)
	terminateReloading()
	reloading.manager = manager

	C.serviceFinderAsync(SERVICE_NAME, PORT, onReload, QUERY_INTERVAL)
end

function global.terminateReloading()
	if reloading.connection then 
		reloading.connection:close()
		reloading.connection = nil
	end	

	reloading = { }
end

function global.updateReloading()
	if reloading.connection then
		local inStream = reloading.connection.inStream;	
		inStream:receive()	
		if inStream:dataLength() > 0 then
			reloadItem()
			return true
		end
	end

	return false
end