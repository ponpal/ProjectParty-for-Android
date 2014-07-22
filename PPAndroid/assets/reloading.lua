local reloading = { }

local SERVICE_NAME = "RELOADING_SERVICE"
local PORT = 22222
local QUERY_INTERVAL = 1000


local function onReload(event, success)
	if success then 
		local buffer = event.buffer

		local ip 	= C.bufferReadInt(buffer)
		local port 	= C.bufferReadShort(buffer)
		local sock = TcpSocket(1024, 128)
		local res  = sock:connect(ip, port, 1000)

		if res then 
			sock:blocking(false)
			reloading.connection = sock
			sock.outStream:writeString("tower_defence")
			sock.outStream:flush()
		end

		Log.info("Connected to reloading!")
	else
		C.serviceFinderAsync(SERVICE_NAME, PORT, onReload, QUERY_INTERVAL)
	end
end

local function reloadItem()
	local sock = reloading.connection
	local stream = sock.inStream

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
		--We are in the complicated case! Reloading of lua files!
		--Simplest strategy Call Game.stop() reload the file and then Game.restart()
		--It is anying to do this every time :(
		Log.info("reloading lua file!")
		Log.info("Stopping game!")
		Game.stop()

		Log.info("running lua script")
		runExternalFile(Game.name .. "/" .. mainFile)
		Log.info("restarting game")
		Game.restart()
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
		end
	end
end