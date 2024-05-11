function printSmallGuildNames(memberCount)
    -- this method is supposed to print names of all guilds that have less than memberCount max members
    local selectGuildQuery = "SELECT %s FROM guilds WHERE max_members < %d;"
    local selectID = "name"


    -- fixed variable name --
    local result = db.storeQuery(string.format(selectGuildQuery, selectID, memberCount))
    local guildName = result.getString(nameID)
    print(guildName)
end

--[[
	* Fixed incorrect variable name “resultID”
	* Introduced the 'selectID' variable to avoid duplication in case the column ID needs to be changed.
]]--