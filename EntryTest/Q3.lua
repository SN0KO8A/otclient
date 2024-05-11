function RemoveMemberFromPlayersParty(playerId, membername)
    -- fixed creating object --
    local player = Player:new(playerId)
    -- made it local --
    local party = player:getParty()
    
    -- cached members --
    local members = party:getMembers()
    for k,v in pairs(members) do
        -- fixed comparing names --
        if v == membername then
            -- fixed argument (my gues is there is the same comparing as here) --
            party:removeMember(v)
            break
        end
    end
end

--[[
	* Fixed creating object. It is not possible to create an object using Player().
	* Made variables local within the function.
	* Cached members because party:getMembers() is called in each iteration.
	* Fixed comparing names, because classes are compared by reference.
	* Passed v as the member name to party:removeMember(), assuming the same comparison logic is used there.
	* Changed name of function to make it more readable
]]--