function init()
	frame = Loader.loadFrame("orb.png");
	font  = Loader.loadFont("Arial32_0.png", "Arial32.fnt");
    rotation = 0
end

function term()

end

function update()
	rotation = rotation + 0.01
end

function render()
	local pos = Create("vec2", 100, 100);
	local dim = Create("vec2", 103, 190);
	local origin = Create("vec2", 103 / 2, 190 /2);
	Renderer.addFrame(frame, pos, dim, 0xFFFFFFFF);
	pos.y = 300;
	Renderer.addText(font, "Hello little girl!", pos, 0xFF03cc42);
	Renderer.addFrame2(frame, pos, dim, 0xFF00FF00, origin, rotation, true);
end