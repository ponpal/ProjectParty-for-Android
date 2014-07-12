global.Game = { }
global.Input = { }

local discovery
local renderer
local resources
local texture
local font 

local servers = { }
local serverRects = { }
local serverTimers = { }
local serverTimeout = 5
local fileSendingHandle = 0
local receiveFiles = 0
local receivedFiles
local chosenServer = nil

function Game.start()
	discovery = C.serverDiscoveryStart()
	renderer = CRenderer(256)
	resources = C.resourceCreateLocal(10)
    font = C.resourceLoad(resources, "arial50.fnt")
    font = ffi.cast("Font*", font.item)
    Screen.setOrientation(Orientation.portrait)
end

function Game.restart()
	local state = Resources.loadTable("lobby_save_state.luac")
	if state.shouldTransition then

		unbindState()
		C.loadLuaScripts(C.gGame.L, state.gameName)
		Resources.setCLoader(C.resourceCreateNetwork(40, state.gameName))
		Resources.gameName = state.gameName
		Game.restart()
	else 
		Game.start();
	end
end

function Game.stop()

	local toSave = { }
	if chosenServer then
		toSave.gameName = ffi.string(chosenServer.gameName)
		Log.info(toSave.gameName);
		toSave.ip       = chosenServer.serverIP
		toSave.shouldTransition = true
	else 
		toSave.shouldTransition = false
	end 

	Resources.saveTable(toSave, "lobby_save_state.luac")

	C.serverDiscoveryStop(discovery)
	C.resourceUnloadAll(resources)
	C.resourceDestroy(resources)

	unbindState()
end


local function updateServerInfo()
    local info = C.serverNextInfo(discovery)
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
	    	local rect = { }
	    	local dim = C.fontMeasure(font, string.format("%s %s", ffi.string(info.serverName), ffi.string(info.gameName)))
	    	rect.x = 100
	    	rect.y = 100 * #servers
	    	rect.width = dim.x
	    	rect.height = dim.y

	    	table.insert(servers, info)
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
end

local function checkForTransition()
	if receiveFiles ~= 0 then
		local fileStatus = C.receiveFilesStatus(fileSendingHandle)
		if fileStatus == C.TASK_SUCCESS then
			return true
		end
	end

	return false
end

function Game.step()
	updateServerInfo()
	local shouldTransition = checkForTransition()
    if shouldTransition then
		Game.stop()
		C.loadLuaScripts(C.gGame.L, chosenServer.gameName)
		Resources.setCLoader(C.resourceCreateNetwork(40, chosenServer.gameName))
		Resources.gameName = ffi.string(chosenServer.gameName)
		Game.start()
		return;
    end

	gl.glClearColor(1,0,0,1)
    gl.glClear(gl.GL_COLOR_BUFFER_BIT)
    gl.glViewport(0,0,C.gGame.screen.width,C.gGame.screen.height)
    gl.glEnable(gl.GL_BLEND)
    gl.glBlendFunc(gl.GL_SRC_ALPHA, gl.GL_ONE_MINUS_SRC_ALPHA)

	for i, v in ipairs(servers) do 
		local rect = serverRects[i]
		local vec = vec2(rect.x, rect.y)
		renderer:addText(font, string.format("%s %s", ffi.string(v.serverName), ffi.string(v.gameName)), 
			vec, 0xFFFFFFFF)
	end
	
	local msg = ""
	if receiveFiles == 0 then 
		msg = "Connect to a server!"
	else
		local fileStatus = C.receiveFilesStatus(fileSendingHandle)
		if fileStatus == C.TASK_PROCESSING then
			msg = "Processing files"
		elseif fileStatus == C.TASK_SUCCESS then
			msg = "We are done loading files!"
		elseif fileStatus == C.TASK_FAILURE then
			msg = "File loading failures"
			receivedFiles = 1
		end
	end

	renderer:addText(font, msg, vec2(100, Screen.height - 100), 0xFFFFFFFF)
	renderer:draw()
end


local function startReceivingFiles(serverInfo)
	chosenServer = serverInfo
	local dir = ffi.string(C.platformExternalResourceDirectory()).."/"..ffi.string(serverInfo.gameName)
	receiveFiles = C.receiveFiles(serverInfo.serverIP, serverInfo.contentPort, dir)
end

function Input.onDown(pointerID, x, y)
	for i, rect in ipairs(serverRects) do
		if x > rect.x and x < rect.x + rect.width and y > rect.y and y < rect.y + rect.height then
			if receiveFiles == 0 or receivedFiles == 1 then
				startReceivingFiles(servers[i])
				receivedFiles = 2
			end
		end
	end
end

function Input.onMove(pointerID, x, y)
end

function Input.onUp(pointerID, x, y)
end

function Input.onCancel(pointerID, x, y)
end