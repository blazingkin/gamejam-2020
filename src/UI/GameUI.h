#ifndef UI_H
#define UI_H
#include <glm/gtc/type_ptr.hpp>
#define IMGUI_IMPL_OPENGL_LOADER_GLAD 1
#include <imgui/include/imgui.h>
#include <imgui/include/imgui_impl_glfw.h>
#include <imgui/include/imgui_impl_opengl3.h>
#include "../Camera.h"
#include <memory>
#include "../BLZEntity.h"
#include "../Program.h"
using namespace glm;
using namespace std;
#define MAX_SELECTION_RANGE 3

extern glm::vec2 playerMotion;
void handleKey(GLFWwindow *window, int key, int scancode, int action, int mods, BScene *scene);
void handleClick(GLFWwindow *window, int button, int action, int mods, BScene *scene);
void onCursorUpdate(GLFWwindow* window, double xpos, double ypos, BScene *scene);
void updateGameCameraLook(double timeDelta, GLFWwindow *window);
void updateGUI(BScene *scene, float screenWidth, float screenHeight);
void initGUI(GLFWwindow *window);
vec3 projectToPlane(float xpos, float ypos, float width, float height);
#endif