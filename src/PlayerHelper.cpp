#include "PlayerHelper.h"


// mo money, mo problems
int getMoney(BScene *scene) {
    auto player = getPlayer(scene);
    auto playerComponent = player->getComponent<c_player_t>();
    return playerComponent->luck;
}

bool canSpend (BScene *scene, int amount) {
    auto player = getPlayer(scene);
    auto playerComponent = player->getComponent<c_player_t>();
    return playerComponent->luck >= amount;
}

bool attemptCharge(BScene *scene, int amount) {
    auto player = getPlayer(scene);
    auto playerComponent = player->getComponent<c_player_t>();
    bool spend = playerComponent->luck >= amount;
    if (spend) {
        playerComponent->luck -= amount;
    }
    return spend;
}


std::shared_ptr<BLZEntity> getPlayer(BScene *scene) {
    auto players = scene->getEntitiesWithComponents({ COMPONENT_PLAYER });
    if (players.size() != 1) {
        std::cerr << "MORE THAN ONE PLAYER!???" << std::endl;
        return nullptr;
    }
    return players[0];
}