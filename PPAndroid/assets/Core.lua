
cfuns = require("ffi") 

cfuns.cdef[[ 
  	typedef struct { float x; float y; } vec2;

	uint32_t loadFrame(const char* name);
	uint32_t loadFont(const char* frameName, const char* fontName);
	void unloadFrame(uint32_t frame);
	void unloadFont(uint32_t font);
	
	//clock.h
	typedef struct
	{
		uint64_t _totalTime, _lastTime, _elapsedTime;
		bool _isPaused, _isStopped;
	} Clock;


	float clockElapsed(Clock* clock);
	float clockTotal(Clock* clock);


	vec2 measureString(uint32_t font, const char* str);

	void addFrame(uint32_t frame, vec2 pos, vec2 dim, unsigned int color);
	void addFrame2(uint32_t frame, vec2 pos, vec2 dim, unsigned int color,
				  vec2 origin, float rotation, int mirrored);
	void addText(uint32_t font, const char* str, vec2 pos, unsigned int  color);

	void luaLog(const char* toLog);

	//buffer.h
	typedef struct
	{
		uint8_t* base;
		uint8_t* ptr;
		uint32_t length;
	} Buffer;

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
	typedef struct
	{
		Buffer in;
		Buffer out;
	} Network;

	int networkSend(Network* network);
	int networkReceive(Network* network);

	int networkIsAlive(Network* network);
	int networkReconnect(Network* network);

	//game.h
	typedef struct
	{
		Clock* clock;
		Network* network;
		bool paused;

	} Game;

	extern Game* gGame;
]]

Loader = {} 
Loader.loadFrame   = cfuns.C.loadFrame 
Loader.loadFont    = cfuns.C.loadFont 
Loader.unloadFont  = cfuns.C.unloadFont 
Loader.unloadFrame = cfuns.C.unloadFrame

Renderer = {} 
Renderer.addFrame = cfuns.C.addFrame 
Renderer.addFrame2 = cfuns.C.addFrame2 
Renderer.addText   = cfuns.C.addText 

Font = {}
Font.measure =  cfuns.C.measureString;

Network = {}

Time = {}
Time.total   = 0
Time.elapsed = 0


vec2 = cfuns.typeof("vec2")


local gClock = cfuns.C.gGame.clock;
local function updateTime()
	Time.total   = cfuns.C.clockTotal(gClock)
	Time.elapsed = cfuns.C.clockElapsed(gClock)
end


function coreUpdate()
	updateTime();
end