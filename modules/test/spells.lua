spellWindow = nil
jumpButton = nil

windowSizeX = 300
windowSizeY = 300

jumpButtonSizeX = 64
jumpButtonSizeY = 32

marginy = 35
marginx = 15

function init()
	spellWindow = g_ui.displayUI('spells.otui')
	jumpButton = spellWindow:getChildById('jumpButton')
  
	connect(g_app, {onFixedUpdate = onUpdate})
  
	resetJumpPos()
	
	g_game.talk('complete!')
end

function terminate()
	spellWindow.destroy()
end

function toggle()
	g_game.talk('jump!')
	resetJumpPos()
end

function resetJumpPos()
	local windowPos = spellWindow:getPosition()
	local jumpPos = windowPos
	
	local randomY = math.random(windowSizeY - jumpButtonSizeY - marginy)
	
	jumpPos.x = jumpPos.x + (windowSizeX - marginx - jumpButtonSizeX)
	jumpPos.y = jumpPos.y + randomY + marginy
	
	jumpButton:setPosition(jumpPos)
end

function onUpdate()
	if isJumpButtonBeyond() == false then
		updateJumpPos()
	else
		resetJumpPos()
	end
end

function updateJumpPos()
	local jumpPos = jumpButton:getPosition()
	
	jumpPos.x = jumpPos.x - 10
	jumpButton:setPosition(jumpPos)
end

function isJumpButtonBeyond()
	local jumpPos = jumpButton:getPosition()
	local windowPos = spellWindow:getPosition()
	
	local windowBorderX = windowPos.x
	windowBorderX = windowBorderX + marginx
	
	return jumpPos.x < windowBorderX
end