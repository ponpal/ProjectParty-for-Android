local RendMT = { }
RendMT.__index = RendMT

function RendMT:setTransform(transform)
	C.rendererSetTransform(self.cRenderer, transform)
end

function RendMT:addFrame(frame, pos, dim, color)
	C.rendererAddFrame(self.cRenderer, frame, pos, dim, color)
end

function RendMT:addFrameTransform(frame, pos, dim, color, origin, rotation, mirrored)
	C.rendererAddFrame2(self.cRenderer, frame, pos, dim, color, origin, rotation, mirrored)	
end

function RendMT:addText(font, text, pos, color)
	C.rendererAddText(self.cRenderer, font, text, pos, color)
end

function RendMT:draw(matrix)
	if not matrix then
	    matrix = C.matrixOrthogonalProjection(0,C.gGame.screen.width,0,C.gGame.screen.height)
	    if Screen.orientation == Orientation.portrait then
		    matrix = C.matrixTranslate(matrix, C.gGame.screen.width, 0)
		    matrix = C.matrixRotate(matrix, math.pi/2)
		end
	end
	C.rendererSetTransform(self.cRenderer, matrix)
	C.rendererDraw(self.cRenderer)

end

function global.CRenderer(size)
	local t = { }
	setmetatable(t, RendMT)
	t.cRenderer = ffi.gc(C.rendererCreate(size), C.rendererDestroy)
	return t
end


