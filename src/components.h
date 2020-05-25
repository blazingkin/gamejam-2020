#ifndef COMPONENTS_H
#define COMPONENTS_H 1
#include <typeindex>
#include <typeinfo>
#include <glm/gtc/type_ptr.hpp>
#include <memory>
#include "_components.h"
class BLZEntity;
class BScene;
typedef struct _location_obj : component_t {
    glm::vec3 position = glm::vec3(0,0,0);
    glm::vec3 rotationAmount = glm::vec3(0,0,0);
    glm::vec3 size = glm::vec3(1,1,1);
} c_location_t;
#define COMPONENT_LOCATION (std::type_index(typeid(c_location_t)))

typedef void (*collisionFunction)(std::shared_ptr<BLZEntity> self, std::shared_ptr<BLZEntity> other, BScene *scene, float timeDelta);
typedef struct _collider_obj : component_t {
    // Relative to position
    glm::vec3 lowerBound = glm::vec3(0, 0, 0);
    glm::vec3 upperBound = glm::vec3(0, 0, 0);
    collisionFunction collisionReaction = nullptr;
} c_collider_t;
#define COMPONENT_COLLIDER (std::type_index(typeid(c_collider_t)))

#include "RenderableWithPosition.h"
#include "Texture.h"
#include "Material.h"
#include "Shape.h"
#include "Program.h"
#include "BScene.h"
#include <stdint.h>
#include <map>

struct _renderable_obj;


typedef struct _physics_obj : component_t {
    glm::vec3 velocity = glm::vec3(0,0,0);
    float mass = 1;
    uint64_t _dirty = 0;
} c_physics_t;
#define COMPONENT_PHYSICS (std::type_index(typeid(c_physics_t)))

typedef std::shared_ptr<std::vector<Shape>> model_t;

typedef struct _model_obj : component_t {
    glm::vec3 modelOffset = glm::vec3(0, 0, 0);
    glm::vec3 modelResize = glm::vec3(1,1,1);
    glm::vec3 lowerBound = glm::vec3(0, 0, 0);
    glm::vec3 upperBound = glm::vec3(0, 0, 0);
    model_t model = nullptr;
} c_model_t;
#define COMPONENT_MODEL (std::type_index(typeid(c_model_t)))

typedef struct _renderable_obj : component_t {
    std::shared_ptr<Material> material = std::make_shared<Material>();
    std::shared_ptr<Texture> tex = nullptr;
    std::shared_ptr<Texture> normalMap = nullptr;
    std::shared_ptr<Program> program = nullptr;
    std::map<std::string, glm::vec4> shaderUniforms;
    bool drawShadow = false;
    std::vector<std::shared_ptr<RenderableWithPosition>> children = {};
} c_render_t;
#define COMPONENT_RENDERABLE (std::type_index(typeid(c_render_t)))

typedef struct _minimap_renderable_obj : component_t {
} c_minimap_render_t;
#define COMPONENT_MINMAP_RENDERABLE (std::type_index(typeid(c_minimap_render_t)))

typedef struct _minimap_static_obj : component_t {
} c_minimap_static_t;
#define COMPONENT_MINMAP_STATIC (std::type_index(typeid(c_minimap_static_t)))


typedef struct _player_obj : component_t {
    int luck = 0;
    int health = 1;
} c_player_t;
#define COMPONENT_PLAYER (std::type_index(typeid(c_player_t)))

typedef struct _delayed_despawn : component_t {
    double timeLeft = 0;
    double pause = 0;
} c_delayed_despawn_t;
#define COMPONENT_DELAYED_DESPAWN (std::type_index(typeid(c_delayed_despawn_t)))

typedef struct _health : component_t {
	int amount;
} c_health_t;
#define COMPONENT_HEALTH (std::type_index(typeid(c_health_t)))

typedef struct : component_t {
    int bad = 0;
} c_incoming_object_t;
#define COMPONENT_INCOMING_OBJECT (std::type_index(typeid(c_incoming_object_t)))

typedef void(*damageFunction)(std::shared_ptr<BLZEntity> self, std::shared_ptr<BLZEntity> other, BScene *scene);


#endif // Component header guard
