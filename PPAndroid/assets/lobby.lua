Game = { }
Input = { }

function Game.start()
	discovery = C.serverDiscoveryStart()
	renderer = C.rendererInitialize(1024*15)
	C.rendererActivate(renderer)
	resources = C.resourceCreateNetwork(10, "TowerDefence")
    texture = C.resourceLoad(resources, "banana.png")
    font = C.resourceLoad(resources, "ComicSans32.fnt")
    position = vec2(500,500)
    Screen.setOrientation(Orientation.portrait)
end

function Game.restart()

end

function Game.stop()
	C.serverDiscoveryStop(discovery)
	C.resourceUnloadAll(resources)
	C.resourceDestroy(resources)
	C.rendererDestroy(renderer)
end

servers = { }

local frames = 0
local framesElapsed = 0
local fps

local updates = 0
local updatesPerSec = 0
local updatesElapsed = 0

function Game.step()
	frames = frames + 1
	framesElapsed = framesElapsed + C.clockElapsed(C.gGame.clock)
	if framesElapsed > 1 then
		framesElapsed = framesElapsed - 1
		fps = frames
		frames = 0
	end

	updatesElapsed = updatesElapsed + C.clockElapsed(C.gGame.clock)
	if updatesElapsed > 1 then
		updatesElapsed = updatesElapsed - 1
		updatesPerSec = updates
		updates = 0
	end


    gl.glClearColor(1,0,0,1)
    gl.glClear(gl.GL_COLOR_BUFFER_BIT)
    gl.glViewport(0,0,C.gGame.screen.width,C.gGame.screen.height)
    gl.glEnable(gl.GL_BLEND)
    gl.glBlendFunc(gl.GL_SRC_ALPHA, gl.GL_ONE_MINUS_SRC_ALPHA)

    matrix = C.matrixOrthogonalProjection(0,C.gGame.screen.width,0,C.gGame.screen.height)
    matrix = C.matrixTranslate(matrix, C.gGame.screen.width, 0)
    matrix = C.matrixRotate(matrix, math.pi/2)

    info = C.serverNextInfo(discovery)
    if info.serverIP ~= 0 then
    	local exists = false
    	for _, v in ipairs(servers) do
    		if v.serverIP == info.serverIP then 
    			exists = true
    			break
    		end
    	end
    	if not exists then
	    	table.insert(servers, info)
	    end
    end

    frame = Frame(ffi.cast("Texture*", texture.item)[0], 0,0,1,1)
	C.rendererAddFrame(renderer, ffi.new("Frame[1]", frame), position, vec2(50,50), 0xFFFFFFFF)
	for i, v in ipairs(servers) do 
		C.rendererAddText(renderer, ffi.cast("Font*", font.item), v.serverName, vec2(100, 100 * i), 0xFFFFFFFF)
	end
	C.rendererAddText(renderer, ffi.cast("Font*", font.item), string.format("FPS: %f", fps), vec2(100, 500), 0xFFFFFFFF)
	C.rendererAddText(renderer, ffi.cast("Font*", font.item), string.format("TouchHz: %f", updatesPerSec), vec2(100, 400), 0xFFFFFFFF)
	C.rendererDraw(renderer)
	C.rendererSetTransform(renderer, matrix)
end


function Input.onMove(pointerID, x, y)
	C.luaLog(string.format("On move: %d",pointerID))
	position = vec2(x,y)
end

function Input.onDown(pointerID, x, y)
	C.luaLog(string.format("On down: %d",pointerID))
	position = vec2(x,y)
end

function Input.onUp(pointerID, x, y)
	C.luaLog(string.format("On up: %d",pointerID))
	position = vec2(x,y)
end

function Input.onCancel(pointerID, x, y)
	C.luaLog(string.format("On cancel: %d",pointerID))
	position = vec2(x,y)
end
