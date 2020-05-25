#include "SystemIncomingObjects.h"
#include "../PlayerHelper.h"
#include "../Particles.h"

void heartCollision(std::shared_ptr<BLZEntity> self, std::shared_ptr<BLZEntity> other, BScene *scene, float timeDelta) {
    if (other->hasComponent<c_player_t>()) {
        auto playerComponent = other->getComponent<c_player_t>();
        auto playerLocation = other->getComponent<c_location_t>();
        playerComponent->health++;
        self->getComponent<c_collider_t>()->collisionReaction = nullptr;
        self->addComponent<c_delayed_despawn_t>(scene, c_delayed_despawn_t{});
        addParticle(playerLocation->position, glm::vec3(1.0f, 4.2, 1.0), 0.0, 1.2);
        addParticle(playerLocation->position, glm::vec3(1.0f, 6.0, 1.0), 0.0, 1.2);
        addParticle(playerLocation->position, glm::vec3(2.0f, 3.0, 2.0), 0.0, 1.2);
        addParticle(playerLocation->position, glm::vec3(-2.0f, 7.0, -2.0), 0.0, 1.2);
        addParticle(playerLocation->position, glm::vec3(0.0, 8.0, 0.0), 0.0, 1.2);
    }
}

void damageCollision(std::shared_ptr<BLZEntity> self, std::shared_ptr<BLZEntity> other, BScene *scene, float timeDelta) {
    if (other->hasComponent<c_player_t>()) {
        auto playerComponent = other->getComponent<c_player_t>();
        playerComponent->health--;
        self->getComponent<c_collider_t>()->collisionReaction = nullptr;
        self->addComponent<c_delayed_despawn_t>(scene, c_delayed_despawn_t{});
        if (playerComponent->health <= 0) {
            std::cout << "You lost!" << std::endl;
            std::exit(0);
        }
    }
}

void cloverCollision(std::shared_ptr<BLZEntity> self, std::shared_ptr<BLZEntity> other, BScene *scene, float timeDelta) {
    if (other->hasComponent<c_player_t>()) {
        auto playerComponent = other->getComponent<c_player_t>();
        auto playerLocation = other->getComponent<c_location_t>();
        playerComponent->luck += 3;
        if (playerComponent->luck >= config["win_condition_luck"].GetInt()) {
            std::cout << "You win!!!" << std::endl;
            std::exit(0);
        }
        self->getComponent<c_collider_t>()->collisionReaction = nullptr;
        self->addComponent<c_delayed_despawn_t>(scene, c_delayed_despawn_t{});
        auto newSize = playerLocation->size - (glm::vec3(0.0f, 0.0f, playerLocation->size.z) * 0.2f);
        if (newSize.z > 1.5) {
            resize(other, newSize, scene);
        }
        addParticle(playerLocation->position, glm::vec3(1.0f, 4.2, 1.0), 1.0, 1.2);
        addParticle(playerLocation->position, glm::vec3(1.0f, 6.0, 1.0), 1.0, 1.2);
        addParticle(playerLocation->position, glm::vec3(2.0f, 3.0, 2.0), 1.0, 1.2);
        addParticle(playerLocation->position, glm::vec3(-2.0f, 7.0, -2.0), 1.0, 1.2);
        addParticle(playerLocation->position, glm::vec3(0.0, 8.0, 0.0), 1.0, 1.2);
    }
}

bool luckCheck(int luck) {
    return (rand() % config["odds"].GetInt()) < luck;
}

void SpawnObjects(BScene *scene, double timeDelta) {
    int r = rand() % 3;
    static double timeElapsed = 0;
    timeElapsed += timeDelta;
    auto player = getPlayer(scene)->getComponent<c_player_t>();
    while (timeElapsed > config["incoming_object"]["cooldown"].GetDouble()) {
        timeElapsed -= config["incoming_object"]["cooldown"].GetDouble();
        std::shared_ptr<BLZEntity> heart, clover, spike;
        auto size = glm::vec3(config["incoming_object"]["size"]["x"].GetFloat(), config["incoming_object"]["size"]["y"].GetFloat(), config["incoming_object"]["size"]["z"].GetFloat());
        auto zPosition = rand() % (config["map_values"]["map_size"].GetInt() * 2);
        auto physics = c_physics_t{};
        physics.velocity = glm::vec3(config["incoming_object"]["velocity"]["x"].GetFloat(), 0.0f, config["incoming_object"]["velocity"]["z"].GetFloat());
        auto incoming_object = c_incoming_object_t{};
        zPosition -= config["map_values"]["map_size"].GetInt();
        switch (r) {
            case 0:
                if (luckCheck(player->luck + 1)) {
                    heart = BLZEntity::newEntity(meshes["heart.obj"], scene);
                    heart->addComponent<c_incoming_object_t>(scene, c_incoming_object_t{});
                    heart->getComponent<c_render_t>()->material = materials["redMat"];
                    heart->getComponent<c_collider_t>()->collisionReaction = heartCollision;
                    heart->addComponent<c_physics_t>(scene, physics);
                    resize(heart, size, scene);
                    move(heart, glm::vec3(30, 5, zPosition), scene);
                    setRotationTo(heart, glm::vec3(M_PI / 2, M_PI, M_PI / 2), scene);
                }
            break;
            case 1:
                spike = BLZEntity::newEntity(meshes["flame.obj"], scene);
                incoming_object.bad = 1;
                spike->addComponent<c_incoming_object_t>(scene, incoming_object);
                spike->getComponent<c_render_t>()->material = materials["virus"];
                spike->getComponent<c_collider_t>()->collisionReaction = damageCollision;
                spike->addComponent<c_physics_t>(scene, physics);
                resize(spike, size, scene);
                move(spike, glm::vec3(30, 5, zPosition), scene);
                setRotationTo(spike, glm::vec3(M_PI / 2, M_PI, M_PI / 2), scene);
            break;
            case 2:
                if (luckCheck(player->luck + 3)) {
                    clover = BLZEntity::newEntity(meshes["tree.obj"], scene);
                    clover->addComponent<c_incoming_object_t>(scene, c_incoming_object_t{});
                    clover->getComponent<c_render_t>()->material = materials["grass"];
                    clover->getComponent<c_collider_t>()->collisionReaction = cloverCollision;
                    clover->addComponent<c_physics_t>(scene, physics);
                    resize(clover, size, scene);
                    move(clover, glm::vec3(30, 5, zPosition), scene);
                }
            break;
        }
    }
}



void UpdateObjects(BScene *scene, double timeDelta) {
    auto movingObjects = scene->getEntitiesWithComponent(COMPONENT_INCOMING_OBJECT);
    auto player = getPlayer(scene)->getComponent<c_player_t>();
    for (auto obj : movingObjects) {
        if (obj->hasComponent<c_location_t>()) {
            auto location = obj->getComponent<c_location_t>()->position;
            if (location.y < 0.2 || location.x < -150) {
                scene->RemoveEntity(obj);
            }
        }
    }
}