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
-- @super is nil or a metatable. If super is not nil the type 
-- will inherit from super all functions that override a super 
-- function automatically calls the super function
function Type.define(name, typ, super)
	if not name or not typ then
		error("class is called with the following parameters: class(name, typ, [,super]");
	end

	if RTTI[name] then
		local msg = string.format("Call reloadClass(name, typ) to redefine the class: %s", name)
		error("Call reloadClass to redefine the class ")
	end

	typ.__call = function(...)
		local object = { }
		setmetatable(object, typ)
		if object.init then object:init(...) end
		return object;
	end

	if super then
		type.__SUPER__ = super
		for k, v in pairs(super) do
			if type[k] then
				--Override super function but still call it. 
				local func = type[k]
				type[k] = function (...)
					type.__SUPER__[k](...)
					func(...)
				end
			else
			    type[k] = v
			end
		end
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