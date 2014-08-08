
local bufferMT = { }
bufferMT.__index = bufferMT

function bufferMT.__tostring(self)
	local loc  = tonumber(ffi.cast("int32_t", self))
	local ptr  = tonumber(ffi.cast("int32_t", self.ptr))
	local base = tonumber(ffi.cast("int32_t", self.base)) 
	local len  = self.length
	local cap  = self.capacity


	local used = ptr - base
	local left = len - used

	return string.format("\n\tBuffer %s\n\tLength: %d\n\t Capacity: %d\n\t Pointer: %s\n\t Base: %s Used %s Left %s",
						  loc, len, cap, ptr, base, used, left) 
end

function bufferMT:remaining()
	local rem = C.bufferBytesRemaining(self)
	if C.bufferCheckError(self) then 
		error("Failed checking remaining")
	end

	return rem
end

function bufferMT:consumed()
	local rem = C.bufferBytesConsumed(self)
	if C.bufferCheckError(self) then 
		error("Failed checking consumed")
	end

	return rem
end

function  bufferMT:writeBytes(towrite, length)
	C.bufferWriteBytes(self, towrite, length)
	if C.bufferCheckError(self) then 
		error(string.format("Failed writing bytes %s %d", towrite, length))
	end
end

function  bufferMT:writeByte(byte)
	C.bufferWriteByte(self, byte)
	if C.bufferCheckError(self) then 
		error(string.format("Failed writing byte %d",byte))
	end
end

function  bufferMT:writeShort(short)
	C.bufferWriteShort(self, short)
	if C.bufferCheckError(self) then 
		error(string.format("Failed writing short %d",short))
	end
end

function  bufferMT:writeInt(int)
	C.bufferWriteInt(self, int)
	if C.bufferCheckError(self) then 
		error(string.format("Failed writing int %d",int))
	end
end

function  bufferMT:writeLong(long)
	C.bufferWriteLong(self, long)
	if C.bufferCheckError(self) then 
		error(string.format("Failed writing long %d",long))
	end
end

function  bufferMT:writeFloat(float)
	C.bufferWriteFloat(self, float)
	if C.bufferCheckError(self) then 
		error(string.format("Failed writing float %d",float))
	end
end

function  bufferMT:writeDouble(double)
	C.bufferWriteDouble(self, long)
	if C.bufferCheckError(self) then 
		error(string.format("Failed writing double %d",double))
	end
end

function  bufferMT:writeString(str)
	C.bufferWriteUTF8(self, str)
	if C.bufferCheckError(self) then 
		error(string.format("Failed writing string %s", str))
	end
end

function bufferMT:readString()
	local c_str = C.bufferReadTempUTF8(self)	
	if C.bufferCheckError(self) then 
		error("Failed to readString")
	end

	return ffi.string(c_str)
end

function bufferMT:readByte()
	local res = C.bufferReadByte(self)	
	if C.bufferCheckError(self) then 
		error(string.format("Failed to readByte"))
	end

	return res
end

function bufferMT:readShort()
	local res = C.bufferReadShort(self)	
	if C.bufferCheckError(self) then 
		error(string.format("Failed to readShort"))
	end

	return res
end

function bufferMT:readInt()
	local res = C.bufferReadInt(self)	
	if C.bufferCheckError(self) then 
		error(string.format("Failed to readInt"))
	end

	return res
end

function bufferMT:readLong()
	local res = C.bufferReadLong(self)	
	if C.bufferCheckError(self) then 
		error(string.format("Failed to readLong"))
	end

	return res
end

function bufferMT:readFloat()
	local res = C.bufferReadFloat(self)	
	if C.bufferCheckError(self) then 
		error(string.format("Failed to readFloat"))
	end

	return res
end

function bufferMT:readDouble()
	local res = C.bufferReadDouble(self)	
	if C.bufferCheckError(self) then 
		error(string.format("Failed to readDouble"))
	end

	return res
end

ffi.metatype("Buffer", bufferMT)

function global.Buffer(capacity)
	local buf = ffi.gc(C.bufferNew(capacity), C.bufferDelete)
	return buf
end

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

function ISMT:receive()
	C.streamReceive(self.cHandle)
end

function ISMT:dataLength()
	return C.streamInputDataLength(self.cHandle)
end

function ISMT:buffer()
	return C.streamBuffer(self.cHandle)
end

function global.InputSocketStream(sock, size)
	local t =  { }
	setmetatable(t, ISMT)
	t.cHandle = ffi.gc(C.streamCreate(sock, size, C.INPUT_STREAM), C.streamDestroy)
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

function OSMT:dataLength()
	return C.streamOutputDataLength(self.cHandle)
end 

function OSMT:buffer()
	return C.streamBuffer(self.cHandle)
end

function OSMT:flush()
	C.streamFlush(self.cHandle, true)
	if C.streamCheckError(self.cHandle) then
		error("Failed to flush stream!")
	end 
end 

function global.OutputSocketStream(sock, size)
	local t =  { }
	setmetatable(t, OSMT)
	t.cHandle = ffi.gc(C.streamCreate(sock, size, C.OUTPUT_STREAM), C.streamDestroy)
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

function tcpMT:close()
	C.socketDestroy(self.handle)

	self.handle    = nil
	self.outStream = nil
	self.inStream  = nil
end

function tcpMT:sendTimeout(value)
	C.socketSendTimeout(self.handle, value)
end

function tcpMT:receiveTimeout(value)
	C.socketRecvTimeout(self.handle, value)
end

function tcpMT:connect(ip, port, msecs)
	if not msecs then msecs = 0 end
	local res = C.socketConnect(self.handle, ip, port, msecs)
	if not res then 
		error(string.format("Failed to connect to %d %d", ip, port))
	end
	return res
end

function tcpMT:isAlive()
	return C.socketIsAlive(self.cHandle)
end

function tcpMT:receive()
	self.inStream:receive()
end

function global.TcpSocket(inSize, outSize)
	return TcpFromSocket(C.socketCreate(C.TCP_SOCKET), inSize, outSize)
end

function global.TcpFromSocket(socket, inSize, outSize)
	if not inSize then inSize = 1024 end
	if not outSize then outSize = 1024 end

	local t = { }
	t.handle    = socket
	t.inStream  = InputSocketStream(t.handle,  inSize) 
	t.outStream = OutputSocketStream(t.handle, outSize)
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

function udpMT:close()
	C.socketDestroy(self.handle)

	self.handle 	= nil
	self.inBuffer 	= nil
	self.outBuffer 	= nil
end

function udpMT:sendTimeout(value)
	C.socketSendTimeout(self.handle, value)
end

function udpMT:receiveTimeout(value)
	C.socketRecvTimeout(self.handle, value)
end

function udpMT:connect(ip, port)
	self.ip = ip
	self.port = port
end

function udpMT:receive()
	return C.socketReceive(self.handle, self.inBuffer)
end

function udpMT:send(ip, port)
	if not ip then ip = self.ip end
	if not port then port = self.port end
	return C.socketSend(self.handle, self.outBuffer, ip, port)
end

function udpMT:isAlive()
	return C.socketIsAlive(self.handle)
end

function global.UdpSocket(inSize, outSize)
	if not inSize then inSize = 1024 end
	if not outSize then outSize = 1024 end

	local t = { }
	t.handle 	= C.socketCreate(C.UDP_SOCKET)
	t.inBuffer  = Buffer(inSize)
	t.outBuffer = Buffer(outSize)

	setmetatable(t, udpMT)
	return t
end