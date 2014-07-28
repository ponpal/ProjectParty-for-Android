global.Type = { }
local RTTI = { }
RTTI.__index = RTTI

setmetatable(Type, RTTI)


function Type.define(typ, name)
	if type(typ) ~= "table" or type(name) ~= "string" then 
		error("typ must be a table and name must be a string")
	end

	typ.__index = typ

	if global[name] then 
		return Type.redefine(typ, name)
	end


	global[name] = function(...)
		local object = { }
		setmetatable(object, typ)
		if object.init then object:init(...) end
		return object;
	end

	typ.__TYPE__ = name
	RTTI[name] = typ

	return typ
end

function Type.all(func)
	local seen = { }
	local find_table 
	find_table = function(t, k)
		if seen[t] then return end

		local meta = getmetatable(t)

		if meta then 
			Log.infof("Key has table %s", k)
			func(t, meta) 
		end

		seen[t] = true
		for k, v in pairs(t) do 
			if type(v) == "table" then 
				find_table(v, k)
			end
		end
	end

	find_table(_G, "global")
end


function Type.redefine(typ, name)
	local mt = RTTI[name]

	Log.infof("Redefing type! %s", name)
	Type.all(function (t, meta)
		Log.infof("Redefing type! %s", name)
		Log.infof("%s", meta.__TYPE__)

		if meta.__TYPE__ == name then

			if t.stop then 
				t:stop()			
			end
		end
	end)


	local mt = RTTI[name]
	for k, v in pairs(typ) do
		Log.info(k)
		mt[k] = v
	end

	Type.all(function (t, meta)
		if meta.__TYPE__ == name then 
			if t.restart then 
				t:restart()
			elseif t.start then 
				t:start()
			end
		end
	end)

	return mt
end

function Type.typeName(obj)
	local mt = getmetatable( obj )
	return mt.__TYPE__;
end