
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
	rotation = rotation + 3.14 / 60
end

function render()
	local pos    = vec2(100, 100);
	local dim    = vec2(200, 200);
	local origin = vec2(200 / 2, 200 /2);

	Renderer.addFrame(frame, pos, dim, 0xFFFFFFFF);

	local text = string.format("Total %.2f \nElapsed %.2f" , Time.total, Time.elapsed)
	pos.x = 0;
	pos.y = 0;
	Renderer.addText(font, text, pos, 0xFF03cc42);	
end
