local Res = { }
Res.__index = Res;

local loaders   = { }
local unloaders = { }
local reloaders = { }

function global.addContentType(ext, loader, unloader, reloader)
	loaders[ext]   	= loader
	unloaders[ext] 	= unloader
	reloaders[ext]  = reloader
end

function Res:load(item)
	if self.loaded[item.hash] then
		return self.loaded
	end

	local handle = loaders[item.type](self, item.path)
	handle.__RESOURCE_TYPE_ID__ = item.type;

	self.loaded[item.hash] = handle

	Log.infof("Loaded asset: %s %s %d", handle, item.path, item.hash)
	return handle
end

function Res:reloadFromName(name)
	local base, ext = Path.baseName(name)
	local item = { }
	item.path = Game.name .. "/" .. name
	item.hash = tonumber(base)
	item.type = ext

	Log.infof("Hash: %d", item.hash)

	self:reload(item)
end

function Res:reload(item) 
	local handle = nil
	if self.loaded[item.hash] then
		local func = reloaders[item.type]
		func(self, self.loaded[item.hash], item.path)
		Log.info("Reloaded item: %s", item.path)
	else
		Log.errorf("Reload called on asset that was not loaded: %s", item.path)
	end
end

function Res:unload(item)
	if not self.loaded[item.hash] then return end
	
	local item = self.loaded[item.hash]
	local func = unloaders[item.type]
	func(self, item)
	Log.infof("Unloaded item: %s", item.path)
end

function Res:unloadAll()
	for k,v in pairs(self.loaded) do 
		self.loaded[k] = nil 
		local func = unloaders[v.__RESOURCE_TYPE_ID__]
		func(self, v)
	end
end

function Res:isLoaded(item)
	local hash = 0
	if type(item) == "table" then 
		hash = item.hash
	else
		--Assume int here if not no big deal
		hash = item
	end

	return self.loaded[hash] ~= nil
end


function global.ResourceManager()
	Log.info("Calling ResourceManager ctor")
	local t = { }
	t.loaded = { }
	setmetatable(t, Res)
	return t
end

local function loadLuac(self, path)
	return runExternalFile(path, self)
end 

local function unloadLuac(self, item)
	--NO OP
end

--Might have to get fancier then this!
local function reloadLuac(self, item, item)
	local newItem = loadLuac(self, path)
	for k,v in pairs(newItem) do
		item[k] = v 
	end
end

local function loadPng(self, path)
	local texture = C.loadTexture(path)
	local frame   = FrameRef(Frame(texture[0], 0, 0, 1, 1))
	
	local t = { }
	t.frame = frame
	return t	
end

local function unloadPng(self, item)
	local texture = ffi.new("Texture[1]", item.frame[0].texture)
	C.unloadTexture(texture)
end

local function reloadPng(self, item, path)
	--Ugly temporary 
	local texture = ffi.new("Texture[1]", item.frame[0].texture)
	texture = C.reloadTexture(path, texture)
	item.frame[0].texture = texture[0]
end

local fontMT = { }
fontMT.__index = fontMT;

function fontMT:find(str)
	local fnt = C.fontAtlasFindFont(self.font, str)
	assert(fnt ~= nil, string.format("Failed to find font: %s", str))
	return fnt;
end

local function loadFnt(self, path)
	local fnt = C.loadFont(path)
	local t = { }
	setmetatable(t, fontMT);
	t.font = fnt
	return t
end

local function unloadFnt(self, item)
	C.unloadFont(item.font)
end

local function reloadFnt(self, item, path)
	item.font = C.reloadFont(path, item.font)
end

local function loadAtl(self, path)
	local tex = C.loadTexture(Path.changeExt(path, "png"))
	local atl = runExternalFile(path)

	for k, v in pairs(atl) do
		atl[k] = FrameRef(Frame(tex, v[1], v[2], v[3], v[4]))
	end

	atl.__TEXTURE__ = tex
	return atl
end

local function unloadAtl(self, item)
	C.unloadTexture(item.__TEXTURE__)
end

local function reloadAtl(self, item, path)
	local tex = C.reloadTexture(Path.changeExt(path, "png"), item.__TEXTURE__)
	local atl = runExternalFile(path, tex)

	item.__TEXTURE__ = tex
	item.width  = atl.width
	item.height = atl.height
	for k, v in pairs(atl) do
		if item[k] then 
			item[k][0].texture = tex
			item[k][0].x	   = v[1]
			item[k][0].y	   = v[2]
			item[k][0].width   = v[3]
			item[k][0].height  = v[4]
		else 
			item[k] = v
		end
	end
end

addContentType("luac", loadLuac, unloadLuac, reloadLuac)
addContentType("png",  loadPng,  unloadPng,  reloadPng)
addContentType("fnt",  loadFnt,  unloadFnt,  reloadFnt)
addContentType("atl",  loadAtl,  unloadAtl,  reloadAtl)