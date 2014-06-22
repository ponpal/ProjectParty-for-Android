local ScreenStackMT = {}
ScreenStackMT.__index = {
	pop = function(self)
			return table.remove(self.stack)
		end,
	push = function(self, screen)
			table.insert(self.stack, screen)
		end,
	popAll = function(self)
			self.stack = { }
		end,

	update = function(self, ...)
			for i=#self.stack,1,-1 do 
				local screen = self.stack[i]
				screen:update(...)
				if screen.blocksUpdate then
					return
				end
			end
		end,
	render = function(self, ...)
			local blocksRender
			for i=#self.stack, 1, -1 do
				if self.stack[i].blocksRender then
					blocksRender = i
					break
				end
			end
			for i=blocksRender, #self.stack, 1 do
				local screen = self.stack[i]
				screen:render(...)
			end
		end,

	start = function(self, ...)
			for i=i, #self.stack, 1 do
				local screen = self.stack[i]
				screen:start(...)
			end
		end,
	restart = function(self, ...)
			for i=i, #self.stack, 1 do
				local screen = self.stack[i]
				local screenMT = getmetatable(screen)
				if screenMT.restart then 
					screen:restart(...)
				else
					screen:start(...)
				end
			end
		end,
	stop = function(self, ...)
			for i=i, #self.stack, 1 do
				local screen = self.stack[i]
				local screenMT = getmetatable(screen)
				if screenMT.stop then
					screen:stop(...)
				end
			end
		end,

	onDown = function(self, ...)
			for i=#self.stack, 1, -1 do
				local screen = self.stack[i]
				local screenMT = getmetatable(screen)
				if screenMT.onDown then
					screen:onDown(...)
				end
			end
		end,
	onUp = function(self, ...)
			for i=#self.stack, 1, -1 do
				local screen = self.stack[i]
				local screenMT = getmetatable(screen)
				if screenMT.onUp then
					screen:onUp(...)
				end
			end
		end,
	onMove = function(self, ...)
			for i=#self.stack, 1, -1 do
				local screen = self.stack[i]
				local screenMT = getmetatable(screen)
				if screenMT.onMove then
					screen:onMove(...)
				end
			end
		end,
	onCancel = function(self, ...)
			for i=#self.stack, 1, -1 do
				local screen = self.stack[i]
				local screenMT = getmetatable(screen)
				if screenMT.onCancel then
					screen:onCancel(...)
				end
			end
		end
}

function ScreenStack()
	local t = { }
	t.stack = { }
	setmetatable(t, ScreenStackMT)
	return t
end
