#ifndef PLAYER_HELPER_H
#define PLAYER_HELPER_H
#include "BScene.h"
#include "BLZEntity.h"
#include "components.h"
#include <iostream>

int getMoney(BScene *scene);
bool canSpend (BScene *scene, int amount);
bool attemptCharge(BScene *scene, int amount);
std::shared_ptr<BLZEntity> getPlayer(BScene *scene);    

#endif