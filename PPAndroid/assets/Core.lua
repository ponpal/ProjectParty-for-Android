
cfuns = require("ffi") 

cfuns.cdef[[ 
  	typedef struct { float x; float y; } vec2;

	unsigned int loadFrame(const char* name);
	unsigned int loadFont(const char* frameName, const char* fontName);
	void unloadFrame(unsigned int frame);
	void unloadFont(unsigned int font);
	

	void addFrame(unsigned int frame, vec2 pos, vec2 dim, unsigned int color);
	void addFrame2(unsigned int frame, vec2 pos, vec2 dim, unsigned int color,
				  vec2 origin, float rotation, int mirrored);
	void addText(unsigned int font, const char* str, vec2 pos, unsigned int  color);
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

Create = cfuns.new 