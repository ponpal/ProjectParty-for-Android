local sensorNetworkID = 1
local deathNetworkID  = 50
local score = 0


--Timing related stuff below
local fpsCounter = 0
local fps = 0
local lastSecond = 0

local function updateTime()
	if Time.total - lastSecond > 1 then
		fps = fpsCounter
		fpsCounter = 0
		lastSecond = Time.total
	end
	fpsCounter = fpsCounter + 1
end

local function renderTime()
	local pos  = vec2(0,0);
	local text = string.format("FPS: %d Total %.2f \nElapsed %.3f" , fps, Time.total, Time.elapsed)
	Renderer.addText(font, text, pos, 0xFF03cc42);	
end

function init()
	frame = Loader.loadFrame("orb.png")
	font  = Loader.loadFont("Arial32_0.png", "Arial32.fnt")
    rotation = 0

    cfuns.C.networkConnect(Network);
end

function term()
	Loader.unloadFrame(frame)
	Loader.unloadFont(font)

	cfuns.C.networkShutdown(Network);
end

function handleMessage(id, length)

	if id == deathNetworkID then
		score = In.readShort(Network.in_)
	end
end

function update()
	updateTime();

	if not cfuns.C.networkIsAlive(Network) then
		log("We are not connected :(")
		return;
	end

	local b = Network.out
	--Send accelerometer data. (This should be in C/C++?)
	
	if useButtons then
		--Send le buttons
	else 
		Out.writeShort(b, 25)
		Out.writeByte (b, sensorNetworkID)
		Out.writeVec3(b, Sensors.acceleration)
		Out.writeVec3(b, Sensors.gyroscope)
	
	end


	cfuns.C.networkSend(Network)
end

function render()
	local pos = vec2(0, 0);
	local text = string.format("Score: %d", score)
	local dim = vec2(480, 800)

	Renderer.addFrame(frame, pos, dim, 0xFFFFFFFF)

	pos.y = 700
	Renderer.addText(font, text, pos, 0xFF00cccc)

	renderTime()
end

