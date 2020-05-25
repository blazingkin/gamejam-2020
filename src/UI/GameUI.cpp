#include "GameUI.h"
#include <GLFW/glfw3.h>
#include "../PlayerHelper.h"

bool isPlacingTower = false;
GLFWwindow *gameWindow = nullptr;
float gTrans = 0;
int dummyMat = 0;
float moveSpeed = 0.3f;
vec2 playerMotion = vec2(0);
std::shared_ptr<BLZEntity> uiPhantomTower = nullptr;
std::shared_ptr<BLZEntity> uiTowerRadius = nullptr;

std::shared_ptr<BLZEntity> selectedEntity = nullptr;
std::shared_ptr<Program> selectedEntityPreviousProgram = nullptr;

std::shared_ptr<Program> selectedEntityShader;
std::shared_ptr<Program> selectedTowerRadius;
void InitUIShaders() {

}


void handleKey(GLFWwindow *window, int key, int scancode, int action, int mods, BScene *scene)
{
    if (key == GLFW_KEY_ESCAPE && (action == GLFW_PRESS )) {
     
    }

    if (key == GLFW_KEY_ESCAPE && action == GLFW_REPEAT) {

        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    if (key == GLFW_KEY_LEFT_SHIFT && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        globalCamera->eye.y -= 0.2;
        globalCamera->lookAt.y -= 0.2;
    } 
    if (key == GLFW_KEY_SPACE && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        globalCamera->eye.y +=0.2;
        globalCamera->lookAt.y += 0.2;
    } 
    if (key == GLFW_KEY_W && (action == GLFW_PRESS || action == GLFW_RELEASE)) {
        if (action == GLFW_PRESS){
            playerMotion.y = 1;
        } else {
            playerMotion.y = 0;
        }
    }
    if (key == GLFW_KEY_S && (action == GLFW_PRESS || action == GLFW_RELEASE) ) {
        if (action == GLFW_PRESS) {
            playerMotion.y = -1;
        } else {
            playerMotion.y = 0;
        }
    }
    if (key == GLFW_KEY_D && (action == GLFW_PRESS || action == GLFW_RELEASE)) {
        if (action == GLFW_PRESS) {
            playerMotion.x = 1;
        } else {
            playerMotion.x = 0;
        }
    }
    if (key == GLFW_KEY_A && (action == GLFW_PRESS || action == GLFW_RELEASE)) {
        if (action == GLFW_PRESS) {
            playerMotion.x = -1;					
        } else {
            playerMotion.x = 0;
        }	
    }

    if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    }
    if (key == GLFW_KEY_Z && action == GLFW_RELEASE) {
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    }
}

double mouseDownX, mouseDownY;
bool mouseIsDown = false;
float hcameraSpeed = 5.5;
float vcameraSpeed = 0.0f;
float vTheta = -M_PI / 2;
float vPhi = 0;
void handleClick(GLFWwindow *window, int button, int action, int mods, BScene *scene) {
    auto io = ImGui::GetIO();
    if (io.WantCaptureMouse) {
        return;
    }
    // Get window size and mouse position
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    if (width <= 0 || height <= 0) {
        return;
    }
    double posX, posY;
    if (button == GLFW_MOUSE_BUTTON_RIGHT && GLFW_PRESS == action)
    {
        mouseDownX = posX;
        mouseDownY = posY;
        mouseIsDown = true;
    } else if (button == GLFW_MOUSE_BUTTON_RIGHT && GLFW_RELEASE == action) {
        mouseIsDown = false;
    }
}

void onCursorUpdate(GLFWwindow* window, double xpos, double ypos, BScene *scene){
    // Get the mouse position
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    if (width <= 0 || height <= 0) {
        return;
    }

   
}

void updateGameCameraLook(double timeDelta, GLFWwindow *window){
    if (mouseIsDown) {

    }
}

void updateGUI(BScene *scene, float screenWidth, float screenHeight) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    auto player = getPlayer(scene)->getComponent<c_player_t>();
    ImGui::Begin("Luck");
    static bool startup = true;
    if (startup) {
        ImGui::SetWindowPos(ImVec2{0,0});
        ImGui::SetWindowSize(ImVec2{screenWidth / 4, screenHeight / 4});
        startup = false;
    }

    ImGui::Text("Luck: %d", player->luck);
    ImGui::Text("Health: %d", player->health);

    ImGui::NewLine();
    ImGui::NewLine();
    ImGui::NewLine();
    ImGui::Text("Only %d more luck to go!", config["win_condition_luck"].GetInt() - player->luck);

	ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}


void initGUI(GLFWwindow *window) {
    gameWindow = window;
        // Decide GL+GLSL versions
    #if __APPLE__
        // GL 3.2 + GLSL 150
        const char* glsl_version = "#version 150";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
    #else
        // GL 3.0 + GLSL 130
        const char* glsl_version = "#version 130";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
        //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
    #endif
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    #if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
        bool err = gl3wInit() != 0;
    #elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
        bool err = glewInit() != GLEW_OK;
    #elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
        bool err = gladLoadGL() == 0;
    #elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
        bool err = false;
        glbinding::Binding::initialize();
    #elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
        bool err = false;
        glbinding::initialize([](const char* name) { return (glbinding::ProcAddress)glfwGetProcAddress(name); });
    #else
        bool err = false; // If you use IMGUI_IMPL_OPENGL_LOADER_CUSTOM, your loader is likely to requires some form of initialization.
    #endif

    if (err)
    {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return;
    }
    
    // IMGUI setup
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    InitUIShaders();
}


vec3 projectToPlane(float xpos, float ypos, float width, float height) {
        float aspect = width/(float)height;

    // Do the inverse matrix transforms to get back to world space
    vec2 mouse = vec2(xpos, height - 1.0 - ypos);
    auto screenPosNear = vec3(mouse.x, mouse.y, 0);
    auto screenPosFar = vec3(mouse.x, mouse.y, 10000);

    auto P = perspective(45.0f, aspect, 0.01f, 10000.0f);
    auto V = lookAt(globalCamera->eye, globalCamera->lookAt, globalCamera->up);

    auto near_this = unProject(screenPosNear, V, P, vec4(0, 0, width, height));
    auto far_this = unProject(screenPosFar, V, P, vec4(0, 0, width, height));
    auto dir = far_this - near_this;

    // Calculate the intersection with the base plane
    float planeHeight = 0;
    dir /= abs(dir.y);
    dir *= planeHeight - globalCamera->eye.y;

    vec3 result = globalCamera->eye + vec3(dir);

    return result;
}