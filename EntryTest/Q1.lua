-- added states --
STORAGE_IS_FULL = -1
STORAGE_IS_FILLED = 1

local function releaseStorage(storageID, player)
    player:setStorageValue(storageID, STORAGE_IS_EMPTY)
end

function onLogout(player)
    -- fixed repeating literals --
    storageToCleanID = 1000
    if player:getStorageValue(storageToCleanID) == STORAGE_IS_FILLED then
        -- my guess is here should be one more argument --
        addEvent(releaseStorage, storageToCleanID, player)
    end
    return true
end

--[[
	* I added the variable “storageID” to the function “onLogout” so that if you need to change the storage ID which you want to clean, you only need to change it in one place.
	* I introduced variables to describe the state of storage value: STORAGE_IS_FULL and STORAGE_IS_FILLED. This improves code readability.
	* I suspect that the function “releaseStorage” is missing the storageID argument because it’s being passed into the function “addEvent” as an argument to assign “releaseStorage” function.
]]--