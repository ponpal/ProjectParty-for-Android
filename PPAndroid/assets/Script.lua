local fpsCounter = 0;
local fps = 0;
local lastSecond = 0;

function init()
	frame = Loader.loadFrame("orb.png");
	font  = Loader.loadFont("Arial32_0.png", "Arial32.fnt");
    rotation = 0
end

function term()
	Loader.unloadFrame(frame)
	Loader.unloadFont(font)
end


function update()
	if Time.total - lastSecond > 1 then
		fps = fpsCounter;
		fpsCounter = 0;
		lastSecond = Time.total;
	end
	fpsCounter = fpsCounter + 1;

	rotation = rotation + 3.14 / 60
end

function render()
	local pos    = vec2(100, 100);
	local dim    = vec2(200, 200);
	local origin = vec2(200 / 2, 200 /2);

	Renderer.addFrame(frame, pos, dim, 0xFFFFFFFF);

	local text = string.format("FPS: %f Tötäatal %.2f \nElapsed %.3f" , fps, Time.total, Time.elapsed)
	pos.x = 0;
	pos.y = 0;
	Renderer.addText(font, text, pos, 0xFF03cc42);	
end
