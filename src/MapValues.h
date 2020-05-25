#ifndef MAP_VALUES_H
#define MAP_VALUES_H


#include "Config.h"

//This must be an even number
#define MAP_SIZE config["map_values"]["map_size"].GetInt()
#define BUILDING_SIZE  config["map_values"]["building_size"].GetFloat()
#define MAP_NAME config["map_values"]["map_name"].GetString()

#endif