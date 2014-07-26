
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
	typedef struct Buffer
	{
		uint8_t* base;
		uint8_t* ptr;
		uint32_t length;
		uint32_t capacity;
	} Buffer;


	Buffer* bufferNew(uint32_t bufferSize);
	void bufferDelete(Buffer* buffer);

	Buffer bufferWrapArray(uint8_t* array, uint32_t arraySize);
	uint32_t bufferBytesRemaining(Buffer* buffer);
	uint32_t bufferBytesConsumed(Buffer* buffer);

	//Output buffer
	void bufferWriteBytes(Buffer* buffer, uint8_t* data, uint32_t length);
	void bufferWriteByte(Buffer* buffer, uint8_t data);
	void bufferWriteShort(Buffer* buffer, uint16_t data);
	void bufferWriteInt(Buffer* buffer, uint32_t data);
	void bufferWriteLong(Buffer* buffer, uint64_t data);
	void bufferWriteUTF8(Buffer* buffer, const char* data);
	void bufferWriteFloat(Buffer* buffer, float data);
	void bufferWriteDouble(Buffer* buffer,double data);

	//Input Buffer
	uint32_t bufferReadBytes(Buffer* buffer, uint8_t* dest, uint32_t numBytes);
	char* bufferReadTempUTF8(Buffer* buffer);
	uint32_t bufferReadUTF8(Buffer* buffer, char** dest);
	uint8_t  bufferReadByte(Buffer* buffer);
	uint16_t bufferReadShort(Buffer* buffer);
	uint32_t bufferReadInt(Buffer* buffer);
	uint64_t bufferReadLong(Buffer* buffer);
	float  bufferReadFloat(Buffer* buffer);
	double bufferReadDouble(Buffer* buffer);



	//-----------------------------
	//	    socket_stream.h
	//-----------------------------
	typedef struct SocketStream SocketStream;

	enum
	{
		INPUT_STREAM = 0,
		OUTPUT_STREAM = 1
	};

	SocketStream* streamCreate(int socket, size_t bufferSize, int type);
	void streamDestroy(SocketStream* toDestroy);
	void streamReceive(SocketStream* stream);
	void streamFlush(SocketStream* stream, bool untilFinished);

	uint32_t streamGetPosition(SocketStream* stream);
	void streamPosition(SocketStream* stream, uint32_t position);

	uint32_t streamInputDataLength(SocketStream* stream);
	uint32_t streamOutputDataLength(SocketStream* stream);
	Buffer* streamBuffer(SocketStream* stream);
	bool streamCheckError(SocketStream* stream);

	//Input
	uint8_t streamReadByte(SocketStream* stream);
	uint16_t streamReadShort(SocketStream* stream);
	uint32_t streamReadInt(SocketStream* stream);
	uint64_t streamReadLong(SocketStream* stream);
	float streamFloat(SocketStream* stream);
	double streamDouble(SocketStream* stream);
	size_t streamReadBytes(SocketStream* stream, uint8_t* dest, uint32_t length);
	const char* streamReadTempUTF8(SocketStream* stream);
	const uint8_t* streamReadInPlace(SocketStream* stream, uint32_t length);

	//Output
	void streamWriteByte(SocketStream* stream, uint8_t data);
	void streamWriteShort(SocketStream* stream, uint16_t data);
	void streamWriteInt(SocketStream* stream, uint32_t data);
	void streamWriteLong(SocketStream* stream, uint64_t data);
	void streamWriteFloat(SocketStream* stream, float data);
	void streamWriteDouble(SocketStream* stream, double data);
	void streamWriteBytes(SocketStream* stream, uint8_t* data, uint32_t length);
	void streamWriteUTF8(SocketStream* stream, const char* data);


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


	//----------------------------
	//	resources.h
	//----------------------------
	Texture loadTexture(const char* path);
	void unloadTexture(Texture tex);
	Texture reloadTexture(const char* path, Texture tex);

	Font* loadFont(const char* path);
	void unloadFont(Font* font);
	Font* reloadFont(const char* path, Font* font);


	// ----------------------------
	//  	file_manager.h
	// ----------------------------

	enum 
	{
		TASK_SUCCESS = 0,
		TASK_FAILURE = -1,
		TASK_PROCESSING = 1
	};

	bool receiveFile(SocketStream* stream, const char* fileDirectory, const char* name);
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
	//  	platform.h
	// ----------------------------
	typedef struct
	{
		uint8_t* buffer;
		uint32_t length;
	} Resource;

	int platformVibrate(uint64_t milliseconds);
	uint32_t platformGetBroadcastAddress();
	uint32_t platformLanIP();
	const char* platformDeviceName();
	const char* platformExternalResourceDirectory();
	Resource platformLoadAbsolutePath(const char* name);
	Resource platformLoadResource(const char* path);
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
	
	void rendererAddText(Renderer* renderer, const Font* font,
					 const char* text, vec2f inPos,
					 uint32_t color, float pixels, vec2f thresholds);

	void rendererDraw(Renderer* renderer); // Force a draw.


	// ----------------------------
	//  	lua_core.h
	// ----------------------------
	typedef struct lua_State lua_State;

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

	//------------------------------
	//		service_finder.h
	//------------------------------
	static const uint16_t servicePort = 34299;

	typedef struct
	{
		const char* serviceID;
		Buffer* buffer;
		bool shouldContinue;
	} ServiceEvent;

	typedef void (*foundService)(ServiceEvent*, bool success);
	void serviceFinderAsync(const char* toFind, uint16_t port,
							foundService handler, uint32_t interval);

	//-----------------------------
	//		remote_log.h
	//-----------------------------
	void remoteLog(int verbosity, const char* message);
	void remoteDebugUpdate();

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

	//-----------------------
	// core stuff 
	//-----------------------

	void* malloc(uint32_t);
	void free(void*);

	//-----------------------
	// socket_helpers.h
	//-----------------------
	static const uint32_t TCP_SOCKET = 1;
	static const uint32_t UDP_SOCKET = 2;

	//Socket common
	int socketCreate(int type);
	bool socketBind(int socket, uint32_t ip, uint16_t port);
	void socketSetBlocking(int socket, bool value);
	bool socketIsBlocking(int socket);
	void socketSendTimeout(int socket, uint32_t msecs);
	void socketRecvTimeout(int socket, uint32_t mescs);
	void socketDestroy(int socket);

	//For tcp sockets
	bool socketConnect(int socket, uint32_t ip, uint16_t port, uint32_t msecs);
	bool socketTCPSend(int socket, Buffer* toSend);

	//For udp sockets
	bool socketSend(int socket, Buffer* toSend, uint32_t ip, uint16_t port);
	bool socketReceive(int socket, Buffer* buffer);
]]


C = ffi.C

function vibrate(milliseconds)
	return C.vibrate(milliseconds)
end

Orientation = {}
Orientation.landscape = 0
Orientation.portrait  = 1

Sensors = C.gGame.sensor
Screen  = 
{ 
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
Input = { }

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

function restartGame()
	Game.stop()
	Log.info("stopped the game")

--	C.loadLuaScripts(C.gGame.L, Game.name)
	
	Game.restart()
	Log.info("restarted the game")
end


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

function consoleCall(input)
	local func, err = loadstring(input)
	if not func then 
		return err
	end

	return func()
end


Log = {}

function Log.info(msg)
	C.remoteLog(0, msg)
end

function Log.warn(msg)
	C.remoteLog(1, msg)
end

function Log.error(msg)
	C.remoteLog(2, msg)
end

function Log.infof(fmt, ...)
	C.remoteLog(0, string.format(fmt, ...))
end

function Log.warnf(fmt, ...)
	C.remoteLog(1, string.format(fmt, ...))
end

function Log.errorf(fmt, ...)
	C.remoteLog(2, string.format(fmt, ...))
end

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

function Path.changeExt(path, newExt)
	local tmp = split(path, ".")
	return tmp[#tmp - 1] .. "." .. newExt
end


Game = { }
Game.name = ""
Game.resourceDir = ffi.string(C.platformExternalResourceDirectory()) .. "/"

TextWriter = { }
TextWriter.data = ""
function TextWriter:write(text)
	self.data = self.data .. text;
end

File = { }
local function serialize2(sink, t, tName)
	for k, v in pairs(t) do
		
		local keyStr = nil
		if type(k) == "string" then 
			keyStr = "\"" .. k .. "\""
		else 
			keyStr = tostring(k)
		end

		sink:write(tName)
		sink:write("[")
		sink:write(keyStr)
		sink:write("] = ")

		if type(v) == "table" then
			sink:write("{ }\n")
			serialize2(sink, v, tName .. "[" .. keyStr .. "]")
		elseif type(v) == "string" then
			sink:write("\"" .. v .. "\"")
		else
			sink:write(tostring(v))
		end
		sink:write("\n")
	end
end


function serialize(sink, t, name)
	if type(t) == "table" then 
		sink:write("local ")
		sink:write(name)
		sink:write(" = { }\n")
		serialize2(sink, t, name)
		sink:write("return ")
		sink:write(name)
	else 
		sink:write(name .. " = " .. tostring(t))
	end

	return sink
end 

function File.saveTable(t, name)
	local path = Game.resourceDir .. name
	local file = assert(io.open(path, "w"))
	serialize(file, t, "t")
	assert(file:close())

	Log.infof("Table Saved %s", path)
end

function File.loadTable(name)
	if not Game.name then 
		return runExternalFile(name)
	else 
		return runExternalFile(Game.name .. "/" .. name)
	end
end

function runInternalFile(path)
	local resource = C.platformLoadInternalResource(path)
	local func, err = loadstring(ffi.string(resource.buffer, resource.length))
	C.platformUnloadResource(resource)

	if not func then 
		Log.errorf("Failed to load internal %s Error: %s", path, err)
	else 
		return func()
	end 
end

function runExternalFile(path)
	local resource = C.platformLoadExternalResource(path)
	local str = ffi.string(resource.buffer, resource.length)
	Log.info(str)
	local func, err= loadstring(str)
	C.platformUnloadResource(resource)

	if not func then 
		Log.errorf("Failed to load external %s Error: %s", path, err)
	else 
		return func()
	end 
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
	runInternalFile("R.lua")
	runInternalFile("gl.lua")
	runInternalFile("socket.lua")
	runInternalFile("resources.lua")
	runInternalFile("renderer.lua")
	runInternalFile("lobby.lua")
	runInternalFile("reloading.lua")
end