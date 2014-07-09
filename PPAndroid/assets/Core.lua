
ffi = require("ffi")

ffi.cdef[[

	// ----------------------------
	//  	types.h
	// ----------------------------

	typedef struct { float x; float y; } vec2f;
	typedef struct { float x; float y; float z; } vec3f;
	typedef struct { float x; float y; float z; float w; } vec4f;

	typedef struct { float mat[16]; } matrix4;

	typedef struct {
		uint32_t glName;
		uint32_t width, height;
	} Texture;

	typedef struct {
		Texture texture;
		float x, y, width, height;
	} Frame;


	// ----------------------------
	//  	buffer.h
	// ----------------------------

	typedef struct
	{
		uint8_t* base;
		uint8_t* ptr;
		uint32_t length;
		uint32_t capacity;
	} Buffer;

	Buffer* bufferCreate(size_t size);
	void bufferDestroy(Buffer* buffer);

	uint32_t bufferBytesRemaining(Buffer* buffer);

	void bufferWriteBytes(Buffer* buffer, uint8_t* data, size_t length);
	void bufferWriteByte(Buffer* buffer, uint8_t data);
	void bufferWriteShort(Buffer* buffer, uint16_t data);
	void bufferWriteInt(Buffer* buffer, uint32_t data);
	void bufferWriteLong(Buffer* buffer, uint64_t data);
	void bufferWriteUTF8(Buffer* buffer, const char* data);

	void bufferWriteFloat(Buffer* buffer, float data);
	void bufferWriteDouble(Buffer* buffer,double data);

	size_t bufferReadBytes(Buffer* buffer, uint8_t* dest, size_t numBytes);
	size_t bufferReadUTF8(Buffer* buffer, char** dest);
	uint8_t  bufferReadByte(Buffer* buffer);
	uint16_t bufferReadShort(Buffer* buffer);
	uint32_t bufferReadInt(Buffer* buffer);
	uint64_t bufferReadLong(Buffer* buffer);

	float  bufferReadFloat(Buffer* buffer);
	double bufferReadDouble(Buffer* buffer);


	// ----------------------------
	//  	font.h
	// ----------------------------

	typedef struct
	{
		vec4f textureCoords;
		vec4f srcRect;
		vec2f offset;
		float  advance;
	} CharInfo;

	typedef struct
	{
		Texture page;
		CharInfo* chars;
		size_t charsLength;
		size_t defaultChar;
		float size;
		float base;
		float lineHeight;
	} Font;

	const CharInfo* fontCharInfo(const Font* font, size_t index);

	vec2f fontMeasure(const Font* f, const char* text);


	// ----------------------------
	//  	file_manager.h
	// ----------------------------

	enum {
		TASK_SUCCESS = 0,
		TASK_FAILURE = -1,
		TASK_PROCESSING = 1
	};

	uint32_t receiveFiles(uint32_t ip, uint16_t port, const char* fileDirectory);
	int32_t receiveFilesStatus(uint32_t);


	// ----------------------------
	//  	hash.h
	// ----------------------------

	typedef uint32_t HashID;
	typedef uint16_t ShortHash;

	HashID bytesHash(const void* buf, size_t len, size_t seed);
	ShortHash shortHash(const void* buf, size_t len, size_t seed);


	// ----------------------------
	//  	resource_manager.h
	// ----------------------------

	typedef struct
	{
		HashID hashID;
		HashID typeID;
		void* item;
	}Handle;

	typedef struct
	{
		uint8_t* buffer;
		uint32_t length;
	} Resource;

	struct ResourceManager;

	typedef struct ResourceManager
	{
		const char* resourceDir;
		Resource (*loadResource)(struct ResourceManager* resources, const char* name);
	    Handle* handles;
	    size_t handlesLength;
	} ResourceManager;

	ResourceManager* resourceCreateLocal(size_t numResources);
	ResourceManager* resourceCreateNetwork(size_t numResources, const char* resourceFolder);
	void resourceDestroy(ResourceManager* resources);

	bool resourceIsPathLoaded(ResourceManager* resources, const char* path);
	bool resourceIsHashLoaded(ResourceManager* resources, HashID id);

	Handle* resourceLoad(ResourceManager* resources, const char* path);
	Handle* resourceGetHandle(ResourceManager* resources, HashID id);

	bool resourceUnloadPath(ResourceManager* resources, const char* path);
	bool resourceUnloadHandle(ResourceManager* resources, Handle* handle);

	void resourceUnloadAll(ResourceManager* resources);


	// ----------------------------
	//  	platform.h
	// ----------------------------

	int platformVibrate(uint64_t milliseconds);
	uint32_t platformGetBroadcastAddress();
	const char* platformExternalResourceDirectory();
	Resource platformLoadAbsolutePath(const char* name);
	Resource platformLoadInternalResource(const char* path);
	Resource platformLoadExternalResource(const char* path);
	void platformUnloadResource(Resource resource);
	void platformExit();


	// ----------------------------
	//  	linalg.h
	// ----------------------------

	matrix4 matrixRotate(matrix4 toRotate, float angle);
	matrix4 matrixTranslate(matrix4 toTranslate, float x, float y);
	matrix4 matrixOrthogonalProjection(float left, float right, float bottom, float top);


	// ----------------------------
	//  	new_renderer.h
	// ----------------------------

	typedef struct Renderer Renderer;

	Renderer* rendererCreate(size_t batchSize);
	void rendererDestroy(Renderer* renderer);
	void rendererSetTransform(Renderer* renderer, matrix4 transform);
	void rendererAddFrame(Renderer* renderer, const Frame* frame,
						 vec2f pos, vec2f dim, uint32_t color);
	void rendererAddFrame2(Renderer* renderer, const Frame* frame,
			vec2f pos, vec2f dim, uint32_t color,
			vec2f origin, float rotation, int mirrored);
	void rendererAddText(Renderer* renderer, const Font* font, const char* text, vec2f pos, uint32_t color);
	void rendererDraw(Renderer* renderer); // Force a draw.


	// ----------------------------
	//  	lua_core.h
	// ----------------------------

	typedef struct lua_State lua_State;

    void luaLog(const char* toLog);
    void initializeLuaScripts(const char* scriptsDir);
    void loadLuaScripts(lua_State* L, const char* scriptsDirectory);

	// ----------------------------
	//  	time_helpers.h
	// ----------------------------

	typedef struct
	{
		uint64_t _totalTime, _lastTime, _elapsedTime;
		bool _isPaused, _isStopped;
	} Clock;

	float clockElapsed(Clock* clock);
	float clockTotal(Clock* clock);

	void clockStep(Clock* clock);
	void clockPause(Clock* clock);
	void clockResume(Clock* clock);
	void clockStart(Clock* clock);
	void clockStop(Clock* clock);


	// ----------------------------
	//  	network.h
	// ----------------------------

	typedef struct
	{
		Buffer* in_;
		Buffer* uin;
		Buffer* out;
		Buffer* uout;
		uint32_t remoteIP;
		uint16_t remoteUdpPort;
		int udpSocket;
		int tcpSocket;
		uint64_t sessionID;
		void (*handleMessage)(Buffer* buffer, uint16_t messageID);
	} Network;

    Network* networkCreate(size_t bufferSize);
    void networkDestroy(Network* network);

	int networkSend(Network* network);
	int networkUnreliableSend(Network* network);
	int networkReceive(Network* network);
	int networkUnreliableReceive(Network* network);

	bool networkIsAlive(Network* network);
	int networkConnect(Network* network, uint32_t ip, uint16_t udpPort, uint16_t tcpPort);
	int networkReconnect(Network* network);
	void networkDisconnect(Network* network);

	void networkSendLogMessage(Network* network, const char* message);


	// ----------------------------
	//  	server_discovery.h
	// ----------------------------

	typedef struct ServerDiscovery ServerDiscovery;

	typedef struct {
		char serverName[58];
		char gameName[58];
		uint32_t serverIP;
		uint16_t contentPort;
		uint16_t serverTCPPort;
		uint16_t serverUDPPort;
	} ServerInfo;

	ServerDiscovery* serverDiscoveryStart();
	ServerInfo serverNextInfo(ServerDiscovery* discovery);
	void serverDiscoveryStop(ServerDiscovery* discovery);

	// ----------------------------
	//  	game.h
	// ----------------------------

    typedef struct
    {
        vec3f acceleration;
    } SensorState;

	typedef struct
	{
		uint32_t width, height;
	} Screen;


	typedef struct
	{
		Clock* clock;
		SensorState* sensor;
		Screen* screen;
		uint32_t fps;
		lua_State* L;
	} Game;

	extern Game* gGame;

]]


C = ffi.C

Time = {}
Time.total   = 0
Time.elapsed = 0

Network = {
	send = function()
	    C.networkSend(C.gGame.network)
	end,
  usend = function()
      C.networkUnreliableSend(C.gGame.network)
  end,
	isAlive = function()
		ffi.C.networkIsAlive(C.gGame.network)
	end,
	update = function()
		Network.elapsed = Network.elapsed + Time.elapsed
		if Network.elapsed > Network.sendInterval then
		    Network.elapsed = 0
			Network.send()
		end
	end
}
Network.elapsed = 0
Network.sendInterval = 0.1

Out = { }
 function Out.writeByte(byte)
	C.bufferWriteByte(C.gGame.network.out, byte)
end
function Out.writeShort(short)
	C.bufferWriteShort(C.gGame.network.out, short)
end
function Out.writeInt(int)
	C.bufferWriteInt(C.gGame.network.out, int)
end
function Out.writeLong(long)
	C.bufferWriteShort(C.gGame.network.out, long)
end
function Out.writeFloat(float)
	C.bufferWriteFloat(C.gGame.network.out, float)
end
function Out.writeDouble(double)
	C.bufferWriteDouble(C.gGame.network.out, double)
end
function Out.writeVec2(v)
	Out.writeFloat(v.x)
	Out.writeFloat(v.y)
end
function Out.writeVec3(v)
	Out.writeFloat(v.x)
	Out.writeFloat(v.y)
	Out.writeFloat(v.z)
end
function Out.writeUTF8(s)
	C.bufferWriteUTF8(C.gGame.network.out, s);
end

UOut = { }
 function UOut.writeByte(byte)
	C.bufferWriteByte(C.gGame.network.uout, byte)
end
function UOut.writeShort(short)
	C.bufferWriteShort(C.gGame.network.uout, short)
end
function UOut.writeInt(int)
	C.bufferWriteInt(C.gGame.network.uout, int)
end
function UOut.writeLong(long)
	C.bufferWriteShort(C.gGame.network.uout, long)
end
function UOut.writeFloat(float)
	C.bufferWriteFloat(C.gGame.network.uout, float)
end
function UOut.writeDouble(double)
	C.bufferWriteDouble(C.gGame.network.uout, double)
end

function UOut.writeVec2(v)
	UOut.writeFloat(v.x)
	UOut.writeFloat(v.y)
end

function UOut.writeVec3(v)
	UOut.writeFloat(v.x)
	UOut.writeFloat(v.y)
	UOut.writeFloat(v.z)
end

function UOut.writeUTF8(s)
	C.bufferWriteUTF8(C.gGame.network.out, s);
end


In = { }
In.readByte  = function()
	return C.bufferReadByte(C.gGame.network.in_)
end
In.readShort = function()
	return C.bufferReadShort(C.gGame.network.in_)
end
In.readInt  = function()
	return C.bufferReadInt(C.gGame.network.in_)
end
In.readLong = function()
	return C.bufferReadLong(C.gGame.network.in_)
end
In.readFloat = function()
	return C.bufferReadFloat(C.gGame.network.in_)
end
In.readDouble = function()
	return C.bufferReadDouble(C.gGame.network.in_)
end
In.readVec2 = function()
	return vec2(In.readFloat(), In.readFloat())
end
In.readVec3 = function()
	return vec3(In.readFloat(), In.readFloat(), In.readFloat())
end
In.readUTF8 = function()
	return ffi.string(C.bufferReadLuaString(C.gGame.network.in_))
end

In.readByteArray = function()
	local len = In.readShort()
	local array = {}
	for i=0, len-1, 1 do
		array[i] = In.readByte()
	end
	return array
end

function vibrate(milliseconds)
	return C.vibrate(milliseconds)
end

Orientation = {}
Orientation.landscape = 0
Orientation.portrait  = 1

Sensors = C.gGame.sensor
Screen  = { 
	orientation = Orientation.landscape,
	width  = C.gGame.screen.width,
	height = C.gGame.screen.height
}


function Screen.setOrientation(orientation)
	if orientation == Orientation.portrait then
		Screen.width  = C.gGame.screen.height
		Screen.height = C.gGame.screen.width
	else
		Screen.width  = C.gGame.screen.width
		Screen.height = C.gGame.screen.height
	end
	Screen.orientation = orientation
end

RawInput = { }

local function transformInput(x,y)
	if Screen.orientation == Orientation.portrait then
		return Screen.width - y, Screen.height - x
	else
		return x, Screen.height-y
	end
end

function RawInput.onMove(pointerID, x, y)
	Input.onMove(pointerID, transformInput(x,y))
end

function RawInput.onUp(pointerID, x, y)
	Input.onUp(pointerID, transformInput(x,y))
end

function RawInput.onDown(pointerID, x, y)
	Input.onDown(pointerID, transformInput(x,y))
end

function RawInput.onCancel(pointerID, x, y)
	Input.onCancel(pointerID, transformInput(x,y))
end

function RawInput.onMenuButton()
	Input.onMenuButton()
end

function RawInput.onBackButton()
	Input.onBackButton()
end

Font = ffi.typeof("Font")
FontRef = ffi.typeof("Font[1]")

Frame = ffi.typeof("Frame")
FrameRef = ffi.typeof("Frame[1]")

local vec2_MT =
{
	__add = function (v0, v1)
				return vec2(v0.x + v1.x, v0.y + v1.y)
			end,
	__sub = function (v0, v1)
				return vec2(v0.x - v1.x, v0.y - v1.y)
			end,
	__mul = function (v0, scalar)
				return vec2(v0.x * scalar, v0.y * scalar)
			end,
	__div = function (v0, scalar)
				return vec2(v0.x / scalar, v0.y / scalar)
			end
}

vec2 = ffi.metatype("vec2f", vec2_MT)

local function doNothing(...) end

function unbindState()
	Input.onDown = doNothing
	Input.onUp = doNothing
	Input.onCancel = doNothing
	Input.onMove = doNothing

	Game.step = doNothing
	Game.start = doNothing
	Game.stop = doNothing
	Game.restart = doNothing
end

Resources = {}

Resources.gameName = ""

Path = {}

local function split(string, token)
	local toReturn = { }
	for word in string.gmatch(string, '([^'..token..']+)') do
		table.insert(toReturn, word)
	end
	return toReturn
end

function Path.hasExtension(path, extension)
	local temp = split(path, ".")
	return temp[#temp] == extension
end

function Path.baseName(path)
	local temp = split(path, "/")
	local temp2 = split(temp[#temp], ".")
	return temp2[1], temp2[2]
end

local function saveTable(file, t)
	for k, v in pairs(t) do
		if type(v) == "table" then
			file:write(tostring(k).." = {")
			saveTable(file, v)
			file:write("}")
		else
			file:write(tostring(k).." = "..tostring(v))
		end
		file:write(",")
	end
end

function Resources.saveTable(t, name)
	C.luaLog(string.format("Starting saveTable with name %s", ffi.string(C.platformExternalResourceDirectory())..
					"/"..Resources.gameName.."/"..name))
	local file = assert(io.open(ffi.string(C.platformExternalResourceDirectory())..
					"/"..Resources.gameName.."/"..name, "w"))
	file:write("return {")
	saveTable(file, t)
	file:write("}")
	assert(file:close())
	C.luaLog("Exiting saveTable")
end

function Resources.loadTable(name)
	return runExternalFile(Resources.gameName.."/"..name)
end

function Resources.loadFile(path)
	if Path.hasExtension(path, "luac") then
		local name, ext = Path.baseName(path)
		local hash = C.bytesHash(name, #name, 0)
		local toLoad = Resources.gameName.."/"..tostring(hash).."."..ext
		return runExternalFile(toLoad)
	else
		return C.resourceLoad(Resources.loader, path)
	end
end

function Resources.setCLoader(resourceLoader)
	Resources.loader = resourceLoader
end

function runInternalFile(path)
	local resource = C.platformLoadInternalResource(path)
	local func = loadstring(ffi.string(resource.buffer, resource.length))
	C.platformUnloadResource(resource)
	return func()
end

function runExternalFile(path)
	local resource = C.platformLoadExternalResource(path)
	local func = loadstring(ffi.string(resource.buffer, resource.length))
	C.platformUnloadResource(resource)
	return func()
end

local function unlock_new_index(t, k, v)
  rawset(t, k, v)
end

--===================================================
-- call GLOBAL_unlock(_G)
-- to change things back to normal.
--===================================================
local function GLOBAL_unlock(t)
  local mt = getmetatable(t) or {}
  mt.__newindex = unlock_new_index
  setmetatable(t, mt)
end

local function lock_new_index(t, k, v)
    error("GLOBALS are locked -- " .. k .. " must be declared local, if a global was wanted use global.ident", 2)
end

--===================================================
--=  Niklas Frykholm 
-- basically if user tries to create global variable
-- the system will not let them!!
-- call GLOBAL_lock(_G)
--
--===================================================
local function GLOBAL_lock(t)
  local mt = getmetatable(t) or {}
  mt.__newindex = lock_new_index
  setmetatable(t, mt)
end


local GlobalMT = { }

function GlobalMT.__index(t, k)
	return _G[k]
end

function GlobalMT.__newindex(t, k, v)
	rawset(_G, k, v)
end

global = { }
setmetatable(global, GlobalMT)

do
	GLOBAL_lock(_G)
	runInternalFile("gl.lua")
	runInternalFile("lobby.lua")
	runInternalFile("renderer.lua")
end
