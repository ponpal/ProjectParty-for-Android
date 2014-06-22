local CRendererMT = { }
CRendererMT.__index = { 
	setTransform = function(self, transform)
			C.rendererSetTransform(self.cRenderer, transform)
		end,
	addFrame = function(self, frame, pos, dim, color)
			C.rendererAddFrame(self.cRenderer, frame, pos, dim, color)
		end,
	addFrameTransform = function(self, frame, pos, dim, color, origin, rotation, mirrored)
			C.rendererAddFrame(self.cRenderer, frame, pos, dim, color, origin, rotation, mirrored)
		end,
	addText = function(self, font, text, pos, color)
			C.rendererAddText(self.cRenderer, font, text, pos, color)
		end,
	draw = function(self, matrix)
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
}

function CRenderer(size)
	local t = { }
	setmetatable(t, CRendererMT)
	t.cRenderer = ffi.gc(C.rendererCreate(size), C.rendererDestroy)
	return t
end
