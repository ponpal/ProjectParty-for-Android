--Lobby.lua

local renderer
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

global.resources = ResourceManager()

local function startReceivingFiles(serverInfo)
	if receiveFiles == 0 then
		local dir = ffi.string(C.platformExternalResourceDirectory()) .. "/" .. serverInfo.name
		receiveFiles = C.receiveFiles(serverInfo.ip, serverInfo.contentPort, dir)
	end 
end

local function onServerFound(event, success)
	if success then 
		Log.info("Found a server!")

		local buffer = event.buffer
		--Temporary c_name
		local c_name = C.bufferReadTempUTF8(buffer)

		chosenServer = { }
		chosenServer.name 			= ffi.string(c_name)
		chosenServer.ip   			= C.bufferReadInt(buffer)
		chosenServer.tcpPort   		= C.bufferReadShort(buffer)
		chosenServer.udpPort  		= C.bufferReadShort(buffer)
		chosenServer.contentPort 	= C.bufferReadShort(buffer)
		startReceivingFiles(chosenServer)		
	else 
		--If the service finder operation failed restart it!
   		C.serviceFinderAsync("SERVER_DISCOVERY_SERVICE", C.servicePort, onServerFound, 500)
	end
end

function Game.start()
	renderer = CRenderer(256)
    font = resources:load(R.Fonts)
    Screen.setOrientation(Orientation.portrait)

    C.serviceFinderAsync("SERVER_DISCOVERY_SERVICE", C.servicePort, onServerFound, 500)

end

local function transition(server, shouldRestart)
	unbindState()
	
	Game.name = server.name
	Game.server = server
	Game.resourceDir = Game.resourceDir .. Game.name .. "/"

	initializeReloading(resources)
	C.loadLuaScripts(C.gGame.L, Game.name)

	if shouldRestart then
		Game.restart()
	else
		Game.start()
	end
end

function Game.restart()
	local state = File.loadTable("lobby_save_state.luac")
	if state.shouldTransition then
		transition(state.server, true)
	else 
		Game.start();
	end
end

function Game.stop()
	local toSave = { }
	if chosenServer then
		toSave.server = chosenServer
		toSave.shouldTransition = true
	else 
		toSave.shouldTransition = false
	end 

	File.saveTable(toSave, "lobby_save_state.luac")

	resources:unloadAll()
end


local function checkForTransition()
	if receiveFiles ~= 0 then
		local fileStatus = C.receiveFilesStatus(fileSendingHandle)
		if fileStatus == C.TASK_SUCCESS then
			return true
		elseif fileStatus == C.TASK_FAILURE then 
			receiveFiles = 0
			return false
		end 
	end
	return false
end

function Game.step()
	C.remoteDebugUpdate()
	local shouldTransition = checkForTransition()
    if shouldTransition then
		Game.stop()
		transition(chosenServer, false)
		return;
    end

	gl.glClearColor(0.5,0.5,0.5,1)
    gl.glClear(gl.GL_COLOR_BUFFER_BIT)
    gl.glViewport(0,0,C.gGame.screen.width,C.gGame.screen.height)
    gl.glEnable(gl.GL_BLEND)
    gl.glBlendFunc(gl.GL_SRC_ALPHA, gl.GL_ONE_MINUS_SRC_ALPHA)
	
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

	--Need to scale threshold based upon some value instead!
	local consolas = font:find("consola")
	local dim      = C.fontMeasure(consolas, msg)

	renderer:addText(font:find("consola"), msg, vec2(50, Screen.height - 100), 0xFFFFFFFF, vec2(35, 40), vec2(0.4, 0.5))
	renderer:draw()
end

function Input.onDown(pointerID, x, y)
end

function Input.onMove(pointerID, x, y)
end

function Input.onUp(pointerID, x, y)
end

function Input.onCancel(pointerID, x, y)
end