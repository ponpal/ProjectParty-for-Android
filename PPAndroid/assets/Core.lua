
cfuns = require("ffi") 

cfuns.cdef[[ 

	typedef struct { float x; float y; } vec2;
    typedef struct { float x; float y; float z; } vec3;

	typedef struct
	{
		vec3 acceleration;
		vec3 gyroscope;
	} SensorState;

	typedef struct
	{
		uint64_t _totalTime, _lastTime, _elapsedTime;
		bool _isPaused, _isStopped;
	} Clock;

	typedef struct
	{
		uint8_t* base;
		uint8_t* ptr;
		uint32_t length;
	} Buffer;


	typedef struct
	{
		Buffer* in_;
		Buffer* out;
	} Network;


	//game.h
	typedef struct
	{
		Clock* clock;
		Network* network;
		SensorState* sensors;
		bool paused;

	} Game;

	extern Game* gGame;

	typedef void (*messageHandler)(uint8_t id, uint32_t length);
	extern messageHandler gMessageHandler;


	uint32_t loadFrame(const char* name);
	uint32_t loadFont(const char* frameName, const char* fontName);
	void unloadFrame(uint32_t frame);
	void unloadFont(uint32_t font);
	
	//clock.h
	float clockElapsed(Clock* clock);
	float clockTotal(Clock* clock);


	vec2 measureString(uint32_t font, const char* str);

	void addFrame(uint32_t frame, vec2 pos, vec2 dim, unsigned int color);
	void addFrame2(uint32_t frame, vec2 pos, vec2 dim, unsigned int color,
				  vec2 origin, float rotation, int mirrored);
	void addText(uint32_t font, const char* str, vec2 pos, unsigned int  color);

	void luaLog(const char* toLog);

	//buffer.h
	uint32_t bufferBytesRemaining(Buffer* buffer);

	void bufferWriteBytes(Buffer* buffer, uint8_t* data, size_t length);
	void bufferWriteByte(Buffer* buffer, uint8_t data);
	void bufferWriteShort(Buffer* buffer, uint16_t data);
	void bufferWriteInt(Buffer* buffer, uint32_t data);
	void bufferWriteLong(Buffer* buffer, uint64_t data);
	void bufferWriteUTF8(Buffer* buffer, const char* data);

	void bufferWriteFloat(Buffer* buffer, float data);
	void bufferWriteDouble(Buffer* buffer,double data);

	void bufferReadBytes(Buffer* buffer, uint8_t* dest, size_t numBytes);
	void bufferReadUTF8(Buffer* buffer, const char* dest);
	uint8_t  bufferReadByte(Buffer* buffer);
	uint16_t bufferReadShort(Buffer* buffer);
	uint32_t bufferReadInt(Buffer* buffer);
	uint64_t bufferReadLong(Buffer* buffer);

	float  bufferReadFloat(Buffer* buffer);
	double bufferReadDouble(Buffer* buffer);

	//network.h    
    int networkConnect(Network* network);
    int networkReconnect(Network* network);
    int networkDisconnect(Network* network);

    int networkSend(Network* network);
    int networkReceive(Network* network);

    int networkIsAlive(Network* network);
    int networkShutdown(Network* network);
]]

local C = cfuns.C

log = C.luaLog;

Loader = {} 
Loader.loadFrame   = C.loadFrame 
Loader.loadFont    = C.loadFont 
Loader.unloadFont  = C.unloadFont 
Loader.unloadFrame = C.unloadFrame

Renderer = {} 
Renderer.addFrame  = C.addFrame 
Renderer.addFrame2 = C.addFrame2 
Renderer.addText   = C.addText 

Font = {}
Font.measure =  C.measureString;

Network = C.gGame.network;

Time = {}
Time.total   = 0
Time.elapsed = 0

C.gMessageHandler = function (id, length)
	handleMessage(id, length)
end

Out = { }
Out.writeByte   = C.bufferWriteByte;
Out.writeShort  = C.bufferWriteShort;
Out.writeInt    = C.bufferWriteInt;
Out.writeLong   = C.bufferWriteLong;
Out.writeFloat  = C.bufferWriteFloat;
Out.writeDouble = C.bufferWriteDouble;

Out.writeVec2 = function (b, v)
	C.bufferWriteFloat(b, v.x)
	C.bufferWriteFloat(b, v.y)
end

Out.writeVec3 = function (b, v)
	C.bufferWriteFloat(b, v.x)
	C.bufferWriteFloat(b, v.y)
	C.bufferWriteFloat(b, v.z)
end

In = { }
In.readByte   = C.bufferReadByte;
In.readShort  = C.bufferReadShort;
In.readInt    = C.bufferReadInt;
In.readLong   = C.bufferReadLong;
In.readFloat  = C.bufferReadFloat;
In.readDouble = C.bufferReadDouble


Sensors = C.gGame.sensors

vec2 = cfuns.typeof("vec2")

local gClock = C.gGame.clock;
local function updateTime()
	Time.total   = C.clockTotal(gClock)
	Time.elapsed = C.clockElapsed(gClock)
end

function coreUpdate()
	updateTime();
end