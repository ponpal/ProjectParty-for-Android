function step()
	log()
end

function log()
	Out.writeByte(10)
	Out.writeUTF8("Message from Lua")
	Network.send()
end