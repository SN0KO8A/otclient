void Game::addItemToPlayer(const std::string& recipient, uint16_t itemId)
{
    Player* player = g_game.getPlayerByName(recipient);
    std::unique_ptr<Player> createdPlayerPtr;
    bool isPlayerCreated = false;

    if (!player) {
        player = new Player(nullptr);

        // removed memory leak
        createdPlayerPtr.reset(player);

        if (!IOLoginData::loadPlayerByName(player, recipient)) {
            throw std::runtime_error("ERROR (Add item to player) failed to load player data");
        }
    }

    Item* item = Item::CreateItem(itemId);
    if (!item) {
        throw std::runtime_error("ERROR (Add item to player) item is null");
    }

    g_game.internalAddItem(player->getInbox(), item, INDEX_WHEREEVER, FLAG_NOLIMIT);

    if (player->isOffline()) {
        IOLoginData::savePlayer(player);
    }
}

/*
    * Fixed memory leak by using a smart pointer. This prevents the allocated player from being leaked if loading player data or creating an item fails.
    * Added exception throwing instead of just returning and exiting from the method. This helps understand why this method didn’t add an item to the player.
*/