
cfuns = require("ffi") 

cfuns.cdef[[ 

	typedef struct { float x; float y; } vec2f;
    typedef struct { float x; float y; float z; } vec3f;
    
    typedef void (*touchHandler) (int x, int y, int pointerIndex);
    typedef void (*tapHandler) (int x, int y);
    
	typedef struct
	{
		vec3f acceleration;
		vec3f gyroscope;
		touchHandler onTouch;
		tapHandler   onTap;
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

	typedef struct
	{
		uint32_t width, height;
	} Screen;

	typedef struct Renderer Renderer;
	typedef struct Content  Content;
	typedef struct
	{
		Clock* clock;
		Network* network;
		SensorState* sensor;
		Renderer* renderer;
		Screen* screen;
		Content* content;
		bool paused;
		char* name;

	} Game;

	extern Game* gGame;

	typedef void (*messageHandler)(uint8_t id, uint32_t length);
	extern messageHandler gMessageHandler;


	uint32_t loadFrame(const char* name);
	uint32_t loadFont(const char* fontName);
	void unloadFrame(uint32_t frame);
	void unloadFont(uint32_t font);
	
	//clock.h
	float clockElapsed(Clock* clock);
	float clockTotal(Clock* clock);


	vec2f measureString(uint32_t font, const char* str);

	void addFrame(uint32_t frame, vec2f pos, vec2f dim, unsigned int color);
	void addFrame2(uint32_t frame, vec2f pos, vec2f dim, unsigned int color,
				  vec2f origin, float rotation, int mirrored);
	void addText(uint32_t font, const char* str, vec2f pos, unsigned int  color);

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
	void bufferReadUTF8(Buffer* buffer, const char** dest);
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

function log(s)
	Out.writeShort(#s + 3);
	Out.writeByte(5); 
	Out.writeUTF8(s);
end

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

C.gGame.sensor.onTouch = function (x, y, index) 
	if onTouch then onTouch(x, y, index) end
end

C.gGame.sensor.onTap = function (x, y) 
	if onTap then onTap(x, y) end
end

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
	return C.bufferReadUTF8(C.gGame.network.in_)
end

Sensors = C.gGame.sensor
Screen  = C.gGame.screen;

vec2 = cfuns.typeof("vec2f")

local function updateTime()
	Time.total   = C.clockTotal(C.gGame.clock)
	Time.elapsed = C.clockElapsed(C.gGame.clock)
end

function coreUpdate()
	Sensors = C.gGame.sensor
	Screen  = C.gGame.screen

	updateTime();
end