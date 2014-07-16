local ISMT = { }
ISMT.__index = ISMT
function ISMT:readByte()
	local data = C.streamReadByte(self.cHandle)
	if C.streamCheckError(self.cHandle) then
		error("Failed to read byte!")
	end 
	return data
end 

function ISMT:readShort()
	local data = C.streamReadShort(self.cHandle)
	if C.streamCheckError(self.cHandle) then
		error("Failed to read short!")
	end 
	return data
end 

function ISMT:readInt()
	local data = C.streamReadInt(self.cHandle)
	if C.streamCheckError(self.cHandle) then
		error("Failed to read int!")
	end 
	return data
end 

function ISMT:readLong()
	local data = C.streamReadLong(self.cHandle)
	if C.streamCheckError(self.cHandle) then
		error("Failed to read long!")
	end 
	return data
end 

function ISMT:readFloat()
	local data = C.streamReadFloat(self.cHandle)
	if C.streamCheckError(self.cHandle) then
		error("Failed to read float!")
	end 
	return data
end 

function ISMT:readDouble()
	local data = C.streamReadDouble(self.cHandle)
	if C.streamCheckError(self.cHandle) then
		error("Failed to read float!")
	end 
	return data
end 

function ISMT:readString()
	local c_name = C.streamReadTempUTF8(self.cHandle)
	if C.streamCheckError(self.cHandle) then
		error("Failed to read float!")
	end 
	local data = ffi.string(c_name)
	return data
end 

function ISMT:hasData()
	return C.streamHasInputData(self.cHandle)
end

function global.InputSocketStream(size)
	local t =  { }
	setmetatable(t, ISMT)
	t.cHandle = ffi.gc(C.streamCreate(size), C.streamDestroy)
	return t
end

local OSMT = { }
OSMT.__index = OSMT
function OSMT:writeByte(value)
	C.streamWriteByte(self.cHandle, value)
	if C.streamCheckError(self.cHandle) then
		error("Failed to write byte!")
	end 
end 

function OSMT:writeShort(value)
	C.streamWriteShort(self.cHandle, value)
	if C.streamCheckError(self.cHandle) then
		error("Failed to write short!")
	end 
end 

function OSMT:writeInt(value)
	C.streamWriteInt(self.cHandle, value)
	if C.streamCheckError(self.cHandle) then
		error("Failed to write int!")
	end 
end 

function OSMT:writeLong(value)
	C.streamWriteLong(self.cHandle, value)
	if C.streamCheckError(self.cHandle) then
		error("Failed to write long!")
	end 
end 

function OSMT:writeFloat(value)
	C.streamWriteFloat(self.cHandle, value)
	if C.streamCheckError(self.cHandle) then
		error("Failed to write float!")
	end 
end 

function OSMT:writeDouble(value)
	C.streamWriteDouble(self.cHandle, value)
	if C.streamCheckError(self.cHandle) then
		error("Failed to write double!")
	end 
end 

function OSMT:writeString(value)
	C.streamWriteUTF8(self.cHandle, value)
	if C.streamCheckError(self.cHandle) then
		error("Failed to write utf8!")
	end 
end 

function OSMT:hasData()
	return C.streamHasOutputData(self.cHandle)
end 

function OSMT:flush()
	C.streamFlush(self.cHandle, true)
end 

function global.OutputSocketStream(size)
	local t =  { }
	setmetatable(t, OSMT)
	t.cHandle = ffi.gc(C.streamCreate(size), C.streamDestroy)
	return t
end

local tcpMT = { }
tcpMT.__index = tcpMT;
function tcpMT:bind(ip, port)
	return C.socketBind(self.handle, ip, port)
end

function tcpMT:blocking(value)
	C.socketSetBlocking(self.handle, value)
end

function tcpMT:sendTimeout(value)
	C.socketSendTimeout(self.handle, value)
end

function tcpMT:receiveTimeout(value)
	C.socketRecvTimeout(self.handle, value)
end

function tcpMT:connect(ip, port, msecs)
	if not msecs then msecs = 0 end
	return C.socketConnect(self.handle, ip, port, msecs)
end

local function TcpSocketDtor(socket)
	C.socketDestroy(socket)
end

function global.TcpSocket(inSize, outSize)
	if not inSize then inSize = 1024 end
	if not outSize then outSize = 1024 end

	local t = { }
	t.handle    = C.socketCreate(C.TCP_SOCKET)
	t.inStream  = InputSocketStream(inSize) 
	t.outStream = OutputSocketStream(outSize)
	setmetatable(t, tcpMT)

	--Figure out how to do it with GC here.
	return t
end

local udpMT = { }
udpMT.__index = udpMT
function udpMT:bind(ip, port)
	return C.socketBind(self.handle, ip, port)
end

function udpMT:blocking(value)
	C.socketSetBlocking(self.handle, value)
end

function udpMT:sendTimeout(value)
	C.socketSendTimeout(self.handle, value)
end

function udpMT:receiveTimeout(value)
	C.socketRecvTimeout(self.handle, value)
end

function udpMT:receive()
	return C.socketReceive(self.handle, self.inBuffer)
end

function udpMT:send(ip, port)
	return C.socketSend(self.handle, self.outBuffer, ip, port)
end

function udpMT:sendBuffer(buffer, ip, port)
	return C.socketSend(self.handle, buffer, ip, port)
end

local function UdpSocketDtor(socket)
	C.bufferDestroy(socket.inBuffer)
	C.bufferDestroy(socket.outBuffer)
	C.socketDestroy(socket.handle)
end

function global.UdpSocket(inSize, outSize)
	if not inSize then inSize = 1024 end
	if not outSize then outSize = 1024 end

	local t = { }
	t.handle 	= C.socketCreate(C.UDP_SOCKET)
	t.inBuffer  = C.bufferCreate(inSize)
	t.outBuffer = C.bufferCreate(outSize)
	setmetatable(t, udpMT)
	
	--Need to fix with the GC so that it closes the socket when it can.
	return t
end