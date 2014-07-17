local Res = { }
Res.__index = Res;
function Res:load(path)
	if self.loaded[path] then 
		return self.loaded[path]
	end 

	if Path.endswith("luac", path) then 
		local name, ext = Path.baseName(path)
		local hash = C.bytesHash(name, #name, 0)
		local toLoad = Resources.gameName.."/"..tostring(hash)..".".. ext
		
		local item = runExternalFile(toLoad)
		self.loaded[hash] = runExtern
		return item
	end 

	local handle = C.resourceLoad(self.cHandle, path)
	self.loaded[handle.hash]	
	return handle
end

function Res:unloadHandle(handle)
	return C.resourceUnloadHandle(self.cHandle, handle);
end

function Res:unloadString(path)
	return C.resourceUnloadPath(self.cHandle, path)
end

function Res:reload(path)
	return C.resourceReload(self.cHandle, path)
end

function Res:isLoaded(path)
	return C.resourceIsPathLoaded(self.cHandle, path)
end

function Res:unloadAll()
	C.resourceUnloadAll(self.cHandle)
end

function global.ResourceManager(numResources, resourceFolder)
	Log.info("Calling ResourceManager ctor")
	local t = { }
	setmetatable(t, Res)
	
	if resourceFolder then
		t.cHandle = ffi.gc(C.resourceCreateNetwork(numResources, resourceFolder), 
							 C.resourceDestroy)
	else
		t.cHandle = ffi.gc(C.resourceCreateLocal(numResources), C.resourceDestroy)
	end

	return t
end

local reloading = { }
local function onReload(service, buffer)
	local ip 	= C.bufferReadInt(buffer)
	local port 	= C.bufferReadShort(buffer)

	local sock = TcpSocket(1024, 128)
	local res  = sock:connect(ip, port, 1000)

	if res then 
		sock:blocking(false)
		reloading.connection = sock
		C.streamWriteUTF8(reloading.folder)
	end
end

local function reloadItem()
	local sock = reloading.connection
	local stream = sock.inStream

	sock:blocking(true)
	local name = stream:readString()
	local result = C.receiveFile(stream.cHandle, reloading.folder, name)
	sock:blocking(false)

	if result then 	
		if reloading.manager and reloading.manager:isLoaded(name) then 
			reloading.manager:reload(name)
		end 			
	end 
end

function global.initializeReloading(folder, manager)
	reloading.finder = C.serviceFinderCreate("FILE_RELOADING_SERVICE", 
										     C.servicePort, onReload) 
	reloading.folder = folder
	reloading.manager = manager
end

function global.terminateReloading()
	C.serviceFinderDestroy(reloading.finder)
end

function global.updateReloading()
	if reloading.connection then
		local hasData = reloading.connection:hasData()		
		if hasData then
			reloadItem()
		end
	else 
		local result = C.serviceFinderPollFound(self.finder)
		if not result then
			C.serviceFinderQuery(self.finder)
		end
	end
end