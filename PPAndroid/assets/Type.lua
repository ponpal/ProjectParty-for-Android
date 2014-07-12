global.Type = { }

-- Creates a class from name.
-- @name is the class to create
-- @veriadic the arguments of a 
function Type.create(name, ...)
	local typ = RTTI[name]
	return typ(...)
end

-- Creates a type from (name, type, [,super])
-- @name must be a unique identifier for the class
-- this name will later be used by the RTTI.create(name, [,args]) 
-- method to create this type. 
-- @typ is a metatable
-- If the name is already inuse the name will be redefined. 
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

-- This method redefines a type. This can be usefull 
-- If support for reloading is needed/wanted 
-- By redefining this method all instances of this type
-- will be changed live. 
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