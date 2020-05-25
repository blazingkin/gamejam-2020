#include "SystemDelayedDespawn.h"

void DelayedDespawn(std::vector<std::shared_ptr<BLZEntity>> entities, BScene *scene, float timeDelta) {
    for (auto entity : entities) {
        if (entity->hasComponent<c_physics_t>()) {
            entity->removeComponent<c_physics_t>(scene);
        }
        if (entity->hasComponent<c_collider_t>()) {
            scene->removeFromGrid(entity, *entity->getComponent<c_collider_t>());
            entity->removeComponent<c_collider_t>(scene);
        }
        auto despawnComponent = entity->getComponent<c_delayed_despawn_t>();
        despawnComponent->timeLeft -= timeDelta;
        despawnComponent->pause -= 1;
        if(despawnComponent->pause <= 0)
            resize(entity, entity->getComponent<c_location_t>()->size * 0.95f, scene);
        if (entity->hasComponent<c_render_t>()) {
            auto rendercomponent = entity->getComponent<c_render_t>();
            for (auto childMeshes : rendercomponent->children) {
                childMeshes->relativeLocation.size *= 0.95f;
            }
        }
        if (despawnComponent->timeLeft <= 0) {
            scene->RemoveEntity(entity);
            
        }
    }
}

void DelayedDespawn(BScene *scene, float timeDelta){
    auto temp = scene->getEntitiesWithComponents({COMPONENT_DELAYED_DESPAWN, COMPONENT_LOCATION});
    DelayedDespawn(temp, scene, timeDelta);
}