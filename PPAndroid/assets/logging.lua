
local INFO 		= 0
local WARNING   = 1
local ERROR 	= 2

local function logFormat(level, fmt, ...)
	if logFunction then
		local msg = string.format(fmt, ...)
		logFunction(level, msg)
	end
end

function global.logi(fmt, ...)
	logFormat(INFO, fmt, ...)
end

function global.logw(fmt, ...)
	logFormat(WARNING, fmt, ...)
end

function global.loge(fmt, ...)
	logFormat(ERROR, fmt, ...)
end