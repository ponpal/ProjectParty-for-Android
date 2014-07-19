local discovery
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

local function onServerFound(serviceID, buffer)
	--Temporary c_name
	local c_name = C.bufferReadTempUTF8(buffer); 

	chosenServer = { }
	chosenServer.name 			= ffi.string(c_name);
	chosenServer.ip   			= C.bufferReadInt(buffer)
	chosenServer.tcpPort   		= C.bufferReadShort(buffer)
	chosenServer.udpPort  		= C.bufferReadShort(buffer)
	chosenServer.contentPort 	= C.bufferReadShort(buffer)
end

function Game.start()
	discovery = C.serviceFinderCreate("SERVER_DISCOVERY_SERVICE", C.servicePort, onServerFound)
	renderer = CRenderer(256)
    font = resources:load(R.arial50)
    Screen.setOrientation(Orientation.portrait)
end

local function transition(server)
	unbindState()
	
	Game.name = server.name
	Game.server = server
	Game.resourceDir = Game.resourceDir .. Game.name .. "/"

	initializeReloading(resources)
	C.loadLuaScripts(C.gGame.L, Game.name)
	Game.restart()
end


function Game.restart()
	local state = File.loadTable("lobby_save_state.luac")
	if state.shouldTransition then
		transition(state.server)
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

	C.serviceFinderDestroy(discovery)
	resources:unloadAll()
	unbindState()
end


local function startReceivingFiles(serverInfo)
	if receiveFiles == 0 then
		local dir = ffi.string(C.platformExternalResourceDirectory()) .. "/" .. serverInfo.name
		receiveFiles = C.receiveFiles(serverInfo.ip, serverInfo.contentPort, dir)
	end 
end

local function updateServerInfo()
	C.serviceFinderQuery(discovery)
	local res = C.serviceFinderPollFound(discovery)
	if res then
		startReceivingFiles(chosenServer)
	end
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
	updateServerInfo()
	local shouldTransition = checkForTransition()
    if shouldTransition then
		Game.stop()
		transition(chosenServer)
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

	renderer:addText(font.font, msg, vec2(50, Screen.height - 100), 0xFFFFFFFF)
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