Game = { }
Input = { }

function Game.start()
	discovery = C.serverDiscoveryStart()
	renderer = C.rendererCreate(1024*15)
	resources = C.resourceCreateLocal(10)
    texture = C.resourceLoad(resources, "banana.png")
    font = C.resourceLoad(resources, "arial50.fnt")
    font = ffi.cast("Font*", font.item)
    position = vec2(500,500)
    Screen.setOrientation(Orientation.portrait)
end

local path = ffi.string(C.platformExternalResourceDirectory()).."/lobby_state.luac"
function Game.restart()
	local state = dofile(path)
	if state.shouldTransition then
		unbindState()
		C.loadLuaScripts(C.gGame.L, state.gameName)
	else
		position = state.position
	end
end

function Game.stop()
	C.serverDiscoveryStop(discovery)
	C.resourceUnloadAll(resources)
	C.resourceDestroy(resources)
	C.rendererDestroy(renderer)
	unbindState()
	local f = assert(io.open(path, "w"))
	if chosenServer then
		f:write(string.format([[
			local state = { } 
			state.position = vec2(%d,%d) 
			state.gameName = "%s"
			state.shouldTransition = %s
			return state]],position.x, position.y, ffi.string(chosenServer.gameName), true))
	else
		f:write(string.format([[
			local state = { } 
			state.position = vec2(%d,%d) 
			state.shouldTransition = %s
			return state]],position.x, position.y, false))
	end
	assert(f:close())
end

servers = { }
serverRects = { }
serverTimers = { }
serverTimeout = 5

fileSendingHandle = 0
receiveFiles = 0

chosenServer = nil

function Game.step()
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
    	for i, v in ipairs(servers) do
    		if v.serverIP == info.serverIP then 
    			exists = true
    			serverTimers[i] = 0
    			break
    		end
    	end
    	if not exists then
	    	table.insert(servers, info)
	    	local rect = { }
	    	local dim = C.fontMeasure(font, string.format("%s %s", ffi.string(info.serverName), ffi.string(info.gameName)))
	    	C.luaLog(string.format("Rect x: %d Rect y: %d", dim.x, dim.y))
	    	rect.x = 100
	    	rect.y = 100 * #servers
	    	rect.width = dim.x
	    	rect.height = dim.y
	    	table.insert(serverRects, rect)
	    	table.insert(serverTimers, 0)
	    end
    end

    for i = #serverTimers, 1, -1 do
    	if serverTimers[i] > serverTimeout then
    		table.remove(servers, i)
    		table.remove(serverRects, i)
    		table.remove(serverTimers, i)
    	else
	    	serverTimers[i] = serverTimers[i] + C.clockElapsed(C.gGame.clock)
	    end 
    end

    frame = Frame(ffi.cast("Texture*", texture.item)[0], 0,0,1,1)
	C.rendererAddFrame(renderer, ffi.new("Frame[1]", frame), position, vec2(50,50), 0xFFFFFFFF)
	for i, v in ipairs(servers) do 
		local rect = serverRects[i]
		local vec = vec2(rect.x, rect.y)
		C.rendererAddText(renderer, font, string.format("%s %s", ffi.string(v.serverName), ffi.string(v.gameName)), 
			vec, 0xFFFFFFFF)
	end
	local vec = vec2(100, Screen.height - 100)
	if receiveFiles == 0 then 
		C.rendererAddText(renderer, font, "Connect to a server!", 
			vec, 0xFFFFFFFF)
	else
		local fileStatus = C.receiveFilesStatus(fileSendingHandle)
		if fileStatus == C.TASK_PROCESSING then
			C.rendererAddText(renderer, font, "Processing files", 
				vec, 0xFFFFFFFF)
		elseif fileStatus == C.TASK_SUCCESS then
			C.rendererAddText(renderer, font, "We are done loading files!", 
				vec, 0xFFFFFFFF)
				Game.stop()
				C.loadLuaScripts(C.gGame.L, chosenServer.gameName)
				C.luaLog("Exited loadLuaScripts")
				return
		elseif fileStatus == C.TASK_FAILURE then
			C.rendererAddText(renderer, font, "File loading failures", 
				vec, 0xFFFFFFFF)
			receivedFiles = 1
		end
	end
	C.rendererDraw(renderer)
	C.rendererSetTransform(renderer, matrix)
end

function Input.onMove(pointerID, x, y)
	C.luaLog(string.format("On move: %d",pointerID))
	position = vec2(x,y)
end

local function startReceivingFiles(serverInfo)
	chosenServer = serverInfo
	local dir = ffi.string(C.platformExternalResourceDirectory()).."/"..ffi.string(serverInfo.gameName)
	receiveFiles = C.receiveFiles(serverInfo.serverIP, serverInfo.contentPort, dir)
end

function Input.onDown(pointerID, x, y)
	C.luaLog(string.format("On down: %d",pointerID))
	position = vec2(x,y)
	for i, rect in ipairs(serverRects) do
		if x > rect.x and x < rect.x + rect.width and y > rect.y and y < rect.y + rect.height then
			C.luaLog(string.format("Pressed server: %d", i))
			if receiveFiles == 0 or receivedFiles == 1 then
				startReceivingFiles(servers[i])
				receivedFiles = 2
			end
		end
	end
end

function Input.onUp(pointerID, x, y)
	C.luaLog(string.format("On up: %d",pointerID))
	position = vec2(x,y)
end

function Input.onCancel(pointerID, x, y)
	C.luaLog(string.format("On cancel: %d",pointerID))
	position = vec2(x,y)
end
