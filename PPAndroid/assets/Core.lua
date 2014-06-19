glfuns = require("ffi")

glfuns.cdef[[


    typedef void             GLvoid;
    typedef char             GLchar;
    typedef unsigned int     GLenum;
    typedef unsigned char    GLboolean;
    typedef unsigned int     GLbitfield;
    typedef int8_t   		 GLbyte;
    typedef short            GLshort;
    typedef int              GLint;
    typedef int              GLsizei;
    typedef uint8_t          GLubyte;
    typedef unsigned short   GLushort;
    typedef unsigned int     GLuint;
    typedef float		     GLfloat;
    typedef float		     GLclampf;
    typedef int32_t          GLfixed;

    typedef signed long int	 GLintptr;
    typedef signed long int	 GLsizeiptr;

    void glClear (GLbitfield mask);
    void glClearColor (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
	void glViewport (GLint x, GLint y, GLsizei width, GLsizei height);

]]

gl = glfuns.C

GL_COLOR_BUFFER_BIT = 0x00004000

cfuns = require("ffi")

cfuns.cdef[[

	typedef struct { float x; float y; } vec2f;
    typedef struct { float x; float y; float z; } vec3f;

	typedef struct
	{
		vec3f acceleration;
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
		uint32_t capacity;
	} Buffer;

	typedef struct
	{
		Buffer* in_;
		Buffer* out;
		Buffer* uout;
	} Network;

	typedef struct
	{
		uint32_t width, height;
		uint8_t orientation;
	} Screen;

	typedef struct Renderer Renderer;
	typedef struct
	{
		Clock* clock;
		Network* network;
		SensorState* sensor;
		Renderer* renderer;
		Screen* screen;
		bool paused;
		char* name;
		char* resourceDir;
		uint32_t fps;
	} Game;

	extern Game* gGame;

	//clock.h
	float clockElapsed(Clock* clock);
	float clockTotal(Clock* clock);

	int vibrate(uint64_t milliseconds);

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
	const char* bufferReadLuaString(Buffer* buffer);

	//network.h
    int networkConnect(Network* network);
    int networkReconnect(Network* network);
    int networkDisconnect(Network* network);

    int networkSend(Network* network);
  	int networkUnreliableSend(Network* network);
    int networkReceive(Network* network);

    int networkIsAlive(Network* network);
    int networkShutdown(Network* network);
    
    typedef uint32_t HashID;
        
    typedef struct
    {
        HashID hashID;
        HashID typeID;
        void* item;
    }Handle;

    bool contentIsPathLoaded(const char* path);
    bool contentIsHashLoaded(HashID id);

    Handle* contentLoad(const char* path);
    Handle* contentGetHandle(HashID id);

    bool contentUnloadPath(const char* path);
    bool contentUnloadHandle(Handle* handle);

    void contentUnloadAll();
    
    typedef struct { float mat[16]; } matrix4;

    typedef struct {
        uint32_t glName;
        uint32_t width, height;
    } Texture;

    typedef struct {
        Texture texture;
        float x, y, w, h;
    } Frame;

    matrix4 matrixRotate(matrix4 toRotate, float angle);
    matrix4 matrixTranslate(matrix4 toTranslate, float x, float y);
    matrix4 matrixOrthogonalProjection(float left, float right, float bottom, float top);

	Renderer* rendererInitialize(size_t batchSize);
	void rendererDelete(Renderer* renderer);
	void rendererActivate(Renderer* renderer);
	void rendererSetTransform(Renderer* renderer, matrix4 transform);
	void rendererAddFrame(Renderer* renderer, const Frame* frame,
						 vec2f pos, vec2f dim, uint32_t color);
	void rendererAddFrame2(Renderer* renderer, const Frame* frame,
			vec2f pos, vec2f dim, uint32_t color,
			vec2f origin, float rotation, int mirrored);
	void rendererDraw(Renderer* renderer); // Force a draw.

]]


local C = cfuns.C

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
		cfuns.C.networkIsAlive(C.gGame.network)
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
	return cfuns.string(C.bufferReadLuaString(C.gGame.network.in_))
end



In.readByteArray = function()
	local len = In.readShort()
	local array = {}
	for i=0, len-1, 1 do
		array[i] = In.readByte()
	end
	return array
end

Orientation = {}
Orientation.landscape = 0
Orientation.portrait  = 1

function vibrate(milliseconds)
	return C.vibrate(milliseconds)
end


Sensors = C.gGame.sensor
Screen  = {}

function Screen.setOrientation(orientation)
	if orientation == Orientation.portrait then
		Screen.width  = C.gGame.screen.height
		Screen.height = C.gGame.screen.width
	else
		Screen.width  = C.gGame.screen.width
		Screen.height = C.gGame.screen.height
	end
	C.gGame.screen.orientation = orientation
end

Frame = cfuns.typeof("Frame")

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

vec2 = cfuns.metatype("vec2f", vec2_MT)

local function updateTime()
	Time.total   = C.clockTotal(C.gGame.clock)
	Time.elapsed = C.clockElapsed(C.gGame.clock)
end

function coreUpdate()
	Sensors = C.gGame.sensor

	updateTime();
end

do
	Screen.setOrientation(Orientation.landscape)
	C.gGame.fps = 30
end

Game = {}
function Game.setFps(fps)
	C.gGame.fps = fps
end

function init()
end

function step()
	log("Step start")
	log("Created matrix")
    C.glClearColor(1,0,0,1)
    C.glClear(GL_COLOR_BUFFER_BIT)
    C.glViewport(0,0,C.gGame.screen.width,C.gGame.screen.height)

    matrix = C.matrixOrthogonalProjection(0,C.gGame.screen.width,0,C.gGame.screen.height)

    texture = C.contentLoad("banana.png")

    frame = Frame(cfuns.cast("Texture*", texture.item)[0], 0,0,1,1)
	C.rendererAddFrame(C.gGame.renderer, cfuns.new("Frame[1]", frame), vec2(50,50), vec2(500,500), 0xFFFFFFFF)
	C.rendererDraw(C.gGame.renderer)
	C.rendererSetTransform(C.gGame.renderer, matrix)
end

function log(str)
	Out.writeShort(2+2+#str)
	Out.writeShort(10)
	Out.writeUTF8(str)
	Network.send()
end