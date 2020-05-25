#ifndef SYSTEM_DELAYED_DESPAWN_H
#define SYSTEM_DELAYED_DESPAWN_H
#include "../BLZEntity.h"
#include "../components.h"
#include "../EntityHelper.h"

void DelayedDespawn(std::vector<std::shared_ptr<BLZEntity>> entities, BScene *scene, float timeDelta);
void DelayedDespawn(BScene *scene, float timeDelta);

#endif