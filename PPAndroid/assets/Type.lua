global.Type = { }

function Type.create(name, ...)
	local typ = RTTI[name]
	return typ(...)
end


function Type.define(name, typ)
	if not name or not typ then
		error("class is called with the following parameters: class(name, typ, [,super]");
	end

	if RTTI[name] then
		return Type.redefine(name, typ, super)
	end

	typ.__call = function(...)
		local object = { }
		setmetatable(object, typ)
		if object.init then object:init(...) end
		return object;
	end

	typ.__TYPE__ = name
	global.RTTI[name] = typ
	return typ
end

function Type.redefine(name, typ)
	local mt = RTTI[name]
	for k, v in pairs(typ)
		mt[k] = v
	end

	return mt
end

function Type.typeName(obj)
	local mt = getmetatable( obj )
	return mt.__TYPE__;
end