#pragma once
#ifndef _SCENE_H_
#define _SCENE_H_

#include "Config.h"


#define OUTSIDE_X_BOUNDS config["scene"]["outside_x"].GetInt()
#define OUTSIDE_Z_BOUNDS config["scene"]["outside_z"].GetInt()
#define INSIDE_GRID config["scene"]["inside_grid"].GetInt()

#define NUM_GRID_CELLS config["scene"]["num_grid_cells"].GetInt()

#include <vector>
#include <memory>
#include <set>
#include <algorithm>
#include "Shape.h"
#include "Texture.h"
#include "Program.h"
#include "Skybox.h"
#include "components.h"
#include "MatrixStack.h"
class BLZEntity;
class BScene {
    public:
        BScene();
        BScene(glm::vec2 mapSize);
        void registerEntityComponent(std::shared_ptr<BLZEntity> obj, component_id_t typ);
        void deleteEntityComponent(std::shared_ptr<BLZEntity> obj, component_id_t type);
        void RemoveEntity(std::shared_ptr<BLZEntity> obj);
        
        std::vector<std::shared_ptr<BLZEntity>> getEntitiesWithComponent(component_id_t type);
        std::vector<std::shared_ptr<BLZEntity>> getEntitiesWithComponents(std::vector<component_id_t> types);
        std::shared_ptr<BLZEntity> maybeFindClosestEntityWithComponent(component_id_t type, glm::vec3 position);

        glm::vec3 lightLocation;
        glm::vec3 lightColor = glm::vec3(1, 1, 1);
        std::shared_ptr<Skybox> skybox;

        // Occupancy data structure
        std::vector<std::vector< // A 2d array
        std::vector<std::shared_ptr<BLZEntity>>>> occupancyGrid; // Where each entry is a list of shared pointers of entities
        int posInWorld(glm::vec3 pos);
        void removeFromGrid(std::shared_ptr<BLZEntity> entity, c_collider_t collider);
        void addToGrid(std::shared_ptr<BLZEntity> entity, c_collider_t collider);
        std::set<std::shared_ptr<BLZEntity>> getEntitiesCloseToOrWithinExtents(c_collider_t extents);
        std::tuple<int, int> gridEntryCount;
		glm::vec2 mapSize;
		glm::vec3 occupancyIndexToPos(std::tuple<float, float> pos);
		std::tuple<int, int> posToOccupancyIndex(glm::vec3);
    // Systems
        // Class with public member fields represeting data
        // Static functions within that class that take a list of entities / single entity
    private:
        std::map<component_id_t, std::vector<std::shared_ptr<BLZEntity>>> componentEntityMap;
        uint64_t counter = 1;
        glm::vec2 occupancyGridCellSize;




    
};

extern std::map<std::string, std::shared_ptr<std::vector<Shape>>> meshes;
extern std::map<std::string, std::shared_ptr<Texture>> textures;
//extern std::map<std::string, std::vector<unsigned char>> maps;

#endif