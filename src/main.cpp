/*
 * Program 2 base code - includes modifications to shape and initGeom in preparation to load
 * multi shape objects 
 * CPE 471 Cal Poly Z. Wood + S. Sueda + I. Dunn
 */
extern float vPhi;
extern float vTheta;
#define DR_MP3_IMPLEMENTATION
#include "dr_mp3.h"   /* Enables MP3 decoding. */
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include <iostream>
#include <fstream>
#include <glad/glad.h>
#include <stdio.h>
#include <dirent.h>
#include <ctime>
#include "Camera.h"
#include "GLSL.h"
#include "Program.h"
#include "Shape.h"
#include "BScene.h"
#include "perlin.h"
#include "MatrixStack.h"
#include "WindowManager.h"
#include "AssetHelper.h"
#include "PlayerHelper.h"
#include "UI/GameUI.h"
#include "Utils/ColorConversion.h"
#include "Particles.h"
#include <thread>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader/tiny_obj_loader.h>

// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include "Systems/SystemRender.h"
#include "Systems/SystemPhysics.h"
#include "Systems/SystemDelayedDespawn.h"
#include "Systems/SystemShadows.h"
#include "Systems/SystemIncomingObjects.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include <lodepng/lodepng.h>
#include "MapValues.h"

#include <rapidjson/document.h>
#include "Config.h"

//#include <iostream>
//#include "stb_gen.h"

#define USE_LOADED_MAPS = true;

Document config;

using namespace std;
using namespace glm;

void loadConfig(const string resourceDirectory) {
	cout << "Loading Configs" << endl;
	struct dirent *entry = nullptr;
	DIR *dir = nullptr;
	dir = opendir((resourceDirectory + "/config").c_str());
	if(dir == nullptr){
		std::cerr << "Resource directory did not exist!" << std::endl;
		std::exit(1);
	}
	string content;
	while ((entry = readdir(dir))) {
		if (std::string(entry->d_name).find("global_config.json") != string::npos) {
			ifstream ifs(resourceDirectory + "/config/" + entry->d_name);
			content.assign(istreambuf_iterator<char>(ifs), istreambuf_iterator<char>());
			cout << "Loaded " << entry->d_name << endl;
		}
	}

	//const char* json = "";
	config.Parse(content.c_str());
	
	//cout << config["pillTower"]["health"].GetInt() << endl;

}


void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    ma_decoder* pDecoder = (ma_decoder*)pDevice->pUserData;
    if (pDecoder == NULL) {
        return;
    }

    ma_decoder_read_pcm_frames(pDecoder, pOutput, frameCount);

    (void)pInput;
}



#ifdef _DEBUG_OPENGL
		void GLAPIENTRY
	MessageCallback( GLenum source,
					GLenum type,
					GLuint id,
					GLenum severity,
					GLsizei length,
					const GLchar* message,
					const void* userParam )
	{
	fprintf( stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
			( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ),
				type, severity, message );
	}
#endif

class Application : public EventCallbacks
{

public:

	WindowManager * windowManager = nullptr;
	BScene mainScene;
	// Our shader program
	std::shared_ptr<Program> prog;
	std::shared_ptr<Program> skyboxProg;
	std::shared_ptr<Program> minimapProg;
	std::shared_ptr<Program> loadingScreenProg;
	std::shared_ptr<Program> tintProg;
 
	// Shape to be used (from  file) - modify to support multiple
	shared_ptr<Shape> houseMesh;
	shared_ptr<Shape> boxMesh;
	shared_ptr<Shape> sphereMesh;
	Shape mutatingMesh;
	std::map<std::string, std::shared_ptr<Texture>> uiTextures;


	// Contains vertex information for OpenGL
	GLuint VertexArrayID;

	// Data necessary to give our triangle to OpenGL
	GLuint VertexBufferID;

	//example data that might be useful when trying to compute bounds on multi-shape
	vec3 gMin;

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		handleKey(window, key, scancode, action, mods, &mainScene);
	}

	void scrollCallback(GLFWwindow* window, double deltaX, double deltaY) {

	}

	
	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{
		handleClick(window, button, action, mods, &mainScene);
	}


	void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
	{
		onCursorUpdate(window, xpos, ypos, &mainScene);
	}


	void resizeCallback(GLFWwindow *window, int width, int height)
	{
		if (width <= 0 || height <= 0) {
			return;
		}
		glViewport(0, 0, width, height);
	}

	void initUI(const std::string& resourceDirectory) {
		struct dirent *entry = nullptr;
		DIR *dir = nullptr;
		dir = opendir((resourceDirectory + "/ui/textures").c_str());
		if (dir != nullptr) {
			while ((entry = readdir(dir))) {
				if (std::string(entry->d_name).find(".jpg") != string::npos ||
					std::string(entry->d_name).find(".png") != string::npos) {
					auto tex = new Texture();
					tex->setFilename(resourceDirectory + "/ui/textures/" + entry->d_name);
					tex->init();
					tex->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE); 
					uiTextures[entry->d_name] = make_shared<Texture>(*tex);
				}
			}
		}
	}

	void initScene() {
		globalCamera = new Camera();
		globalCamera->eye = vec3(-MAP_SIZE / 2.0, 60, 0);
		globalCamera->lookAt = vec3((-MAP_SIZE / 2) + config["camera"]["offset_x"].GetFloat(), 60 + config["camera"]["offset_y"].GetFloat(), config["camera"]["offset_z"].GetFloat());
		auto lookDisplacement = globalCamera->lookAt - globalCamera->eye;
		vPhi = asin(lookDisplacement.y / glm::length(lookDisplacement));
		vTheta = atan2(lookDisplacement.z, lookDisplacement.x);
		mainScene = *new BScene(vec2(MAP_SIZE/2, MAP_SIZE/2));
	}
	GLuint quad_VertexArrayID, quad_vertexbuffer;
	void initQuad() {
		//now set up a simple quad for rendering FBO
		glGenVertexArrays(1, &quad_VertexArrayID);
		glBindVertexArray(quad_VertexArrayID);

		static const GLfloat g_quad_vertex_buffer_data[] =
		{
			-1.0f, -1.0f, 0.0f,
			1.0f, -1.0f, 0.0f,
			-1.0f,  1.0f, 0.0f,
			-1.0f,  1.0f, 0.0f,
			1.0f, -1.0f, 0.0f,
			1.0f,  1.0f, 0.0f,
		};

		glGenBuffers(1, &quad_vertexbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW);
	}

	void createFBO(GLuint& fb, GLuint& tex) {
		//initialize FBO

		//set up framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, fb);
		//set up texture
		glBindTexture(GL_TEXTURE_2D, tex);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1920, 1080, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			cout << "Error setting up frame buffer - exiting" << endl;
			exit(0);
		}
	}
	GLuint flashFrameBuf, flashTexBuf, depthBuf;
	
	void init(const std::string& resourceDirectory)
	{
		
		GLSL::checkVersion();

		// Set background color.
		glClearColor(.12f, .34f, .56f, 1.0f);
		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);
		#ifdef _DEBUG_OPENGL
			glEnable(GL_DEBUG_OUTPUT);
			glDebugMessageCallback(MessageCallback, 0);
		#endif

		/* Compile all shaders */
		struct dirent *entry = nullptr;
		DIR *dir = nullptr;
		dir = opendir((resourceDirectory + "/shaders/vertex").c_str());
		if (dir != nullptr) {
			while ((entry = readdir(dir))) {
				if (std::string(entry->d_name).find(".glsl") != string::npos) {
					compile_vertex_shader(resourceDirectory + "/shaders/vertex", entry->d_name);
				}
			}
		}

		dir = opendir((resourceDirectory + "/shaders/fragment").c_str());
		if (dir != nullptr) {
			while ((entry = readdir(dir))) {
				if (std::string(entry->d_name).find(".glsl") != string::npos) {
					compile_fragment_shader(resourceDirectory + "/shaders/fragment", entry->d_name);
				}
			}
		}


		dir = opendir((resourceDirectory + "/skyboxes").c_str());
		if (dir != nullptr) {
			while ((entry = readdir(dir))) {
				if (std::string(entry->d_name) != "." && std::string(entry->d_name) != "..") {
					auto sb = make_shared<Skybox>();
					sb->init(resourceDirectory + "/skyboxes/" + entry->d_name + "/");
					skyboxes[entry->d_name] = sb;
				}
			}
		}



		int unit = 3;
		dir = opendir((resourceDirectory + "/textures").c_str());
		if (dir != nullptr) {
			while ((entry = readdir(dir))) {
				if (std::string(entry->d_name).find(".jpg") != string::npos ||
					std::string(entry->d_name).find(".png") != string::npos) {
					auto tex = new Texture();
					tex->setFilename(resourceDirectory + "/textures/" + entry->d_name);
					tex->init();
					tex->setUnit(unit++); 
					tex->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE); 
					textures[entry->d_name] = make_shared<Texture>(*tex);
				}
			}
		}

		const vector<string> shaders({"tex_vert.glsl", "blinn_phong.glsl"});
		// Initialize the GLSL program.
		prog = make_shared<Program>();
		prog->setVerbose(true);
		prog->init(shaders);
		prog->addUniform("P");
		prog->addUniform("V");
		prog->addUniform("M");
		prog->addUniform("LS");
		prog->addUniform("MatAmb");
		prog->addUniform("MatDif");
		prog->addUniform("MatSpec");
		prog->addUniform("shine");
		prog->addUniform("viewVector");
		prog->addUniform("lightPos");
		prog->addUniform("lightColor");
		prog->addUniform("Texture0");
		prog->addUniform("shadowDepth");
		prog->addUniform("textureEnabled");
		prog->addUniform("normalMap");
		prog->addAttribute("vertPos");
		prog->addAttribute("vertNor");
		prog->addAttribute("vertTex");

		

		const vector<string> skyboxShaders({"sky_vert.glsl", "sky_frag.glsl"});
		skyboxProg = make_shared<Program>();
		skyboxProg->setVerbose(true);
		skyboxProg->init(skyboxShaders);
		skyboxProg->addUniform("P");
		skyboxProg->addUniform("V");
		skyboxProg->addUniform("M");
		skyboxProg->addUniform("skybox");
		skyboxProg->addUniform("lightColor");
		skyboxProg->addAttribute("vertPos");
		skyboxProg->addAttribute("vertTex");


		
		
		const vector<string> tintShaders({"pass_vert.glsl", "flash_red.glsl"});
		tintProg = make_shared<Program>();
		tintProg->setVerbose(true);
		tintProg->init(tintShaders);
		tintProg->addUniform("uTime");
		tintProg->addUniform("health");
		tintProg->addUniform("screen");
		tintProg->addAttribute("vertPos");
		glGenFramebuffers(1, &flashFrameBuf);
		glGenTextures(1, &flashTexBuf);
		createFBO(flashFrameBuf, flashTexBuf);
		glGenRenderbuffers(1, &depthBuf);

		//create one FBO

		//set up depth necessary since we are rendering a mesh that needs depth test
		glBindRenderbuffer(GL_RENDERBUFFER, depthBuf);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 1920, 1080);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuf);
		
		//more FBO set up
		GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
		glDrawBuffers(1, DrawBuffers);
		initQuad();
	}

	void initMaterials(const std::string& resourceDirectory) {
		Material shinyBluePlastic;
			shinyBluePlastic.Ambient = vec3(0.02, 0.04, 0.2);
			shinyBluePlastic.Diffuse = vec3(0, 0.15, 0.9);
			shinyBluePlastic.Specular = vec3(0.14, 0.2, 0.6);
			shinyBluePlastic.shine = 120;
		Material flatGrey;
			flatGrey.Ambient = vec3(0.13, 0.13, 0.14);
			flatGrey.Diffuse = vec3(0.3, 0.3, 0.4);
			flatGrey.Specular = vec3(0.05, 0.05, 0.05);
			flatGrey.shine = 0.2;
		Material brass;
			brass.Ambient = vec3(0.3294, 0.2235, 0.02745);
			brass.Diffuse = vec3(0.7804, 0.5686, 0.11373);
			brass.Specular = vec3(0.9922, 0.941176, 0.80784);
			brass.shine = 27.9;
		Material grass;
			grass.Ambient = vec3(0.0, 0.2235, 0.0);
			grass.Diffuse = vec3(0.1004, 0.686, 0.31373);
			grass.Specular = vec3(0.09922, 0.0941176, 0.080784);
			grass.shine = 1;
		Material tree;
			tree.Ambient = vec3(0.0, 0.2235, 0.0);
			tree.Diffuse = vec3(0.1004, 0.686, 0.31373);
			tree.Specular = vec3(0.03, 0.300, 0.080784);
			tree.shine = 1;
		Material fur;
			fur.Ambient = vec3(50.0/255.0, 30.0/255.0, 15.0/255.0);
			fur.Diffuse = vec3(51.0/255.0, 37.0/255.0, 18.0/255.0);
			fur.Specular = vec3(10.0/255.0, 6.0/255.0, 3.0/255.0);
			fur.shine = 0;
		Material shadow;
			shadow.Ambient = vec3(0);
			shadow.Diffuse = vec3(0);
			shadow.Specular = vec3(0);
			shadow.shine = 0;
		Material straw;
			straw.Ambient = vec3(228.0/500.0, 217.0 / 500.0, 111.0 / 500.0);
			straw.Diffuse = vec3(228.0/500.0, 217.0 / 500.0, 111.0 / 500.0);
			straw.Specular = vec3(0.0, 0.0, 0.0);
			straw.shine = 1;
		Material clothing;
			clothing.Ambient = vec3(0.35, 0.35, 0.35);
			clothing.Diffuse = vec3(0.6, 0.6, 0.6);
			clothing.Specular = vec3(0.05, 0.05, 0.05);
			clothing.shine = 1;
		Material tanSkin;
			tanSkin.Ambient = vec3(0.35, 0.35, 0.35);
			tanSkin.Diffuse = vec3(0.5, 0.5, 0.5);
			tanSkin.Specular = vec3(0.15, 0.15, 0.15);
			tanSkin.shine = 1;
		Material virus;
			virus.Ambient = RGBToVec3(45, 25, 25);
			virus.Diffuse = RGBToVec3(220, 151, 3) - virus.Ambient;
			virus.Specular = vec3(0.2, 0.15, 0.05);
			virus.shine = 8;
		Material houseSiding;
			houseSiding.Ambient = RGBToVec3(60, 60, 40);
			houseSiding.Diffuse = RGBToVec3(180, 176, 126);
			houseSiding.Specular = RGBToVec3(10,10,10);
			houseSiding.shine = 0.6;
		Material houseRoof;
			houseRoof.Ambient = RGBToVec3(5, 40, 70);
			houseRoof.Diffuse = RGBToVec3(12, 141, 122);
			houseRoof.Specular = RGBToVec3(5,5,5);
			houseRoof.shine = 0.4;
		Material towerMaterial;
			towerMaterial.Ambient = RGBToVec3(60, 80, 60);
			towerMaterial.Diffuse = RGBToVec3(108, 150, 108);
			towerMaterial.Specular = RGBToVec3(20,20,20);
			towerMaterial.shine = 8;
		Material redMat;
			redMat.Ambient = vec3(0.2, 0.04, 0.02);
			redMat.Diffuse = vec3(0.9, 0.15, 0);
			redMat.Specular = vec3(0.6, 0.2, 0.12);
			redMat.shine = 23;
		materials["shiny_blue_plastic"]  = make_shared<Material>(shinyBluePlastic);
		materials["flat_grey"] = make_shared<Material>(flatGrey);
		materials["flat_gray"] = make_shared<Material>(flatGrey);
		materials["brass"] = make_shared<Material>(brass);
		materials["grass"] = make_shared<Material>(grass);
		materials["tree"] = make_shared<Material>(tree);
		materials["fur"] = make_shared<Material>(fur);
		materials["shadow"] = make_shared<Material>(shadow);
		materials["straw"] = make_shared<Material>(straw);
		materials["clothing"] = make_shared<Material>(clothing);
		materials["tan_skin"] = make_shared<Material>(tanSkin);
		materials["virus"] = make_shared<Material>(virus);
		materials["house_siding"] = make_shared<Material>(houseSiding);
		materials["house_roof"] = make_shared<Material>(houseRoof);
		materials["tower"] = make_shared<Material>(towerMaterial);
		materials["redMat"] = make_shared<Material>(redMat);

	}
	ma_result mma_result;
	ma_decoder mma_decoder;
	ma_device_config mma_deviceConfig;
	ma_device mma_device;
	int initAudio() {
		
		
		mma_result = ma_decoder_init_file("../resources/music/inspiration.mp3", NULL, &mma_decoder);

		mma_deviceConfig = ma_device_config_init(ma_device_type_playback);
		mma_deviceConfig.playback.format   = mma_decoder.outputFormat;
		mma_deviceConfig.playback.channels = mma_decoder.outputChannels;
		mma_deviceConfig.sampleRate        = mma_decoder.outputSampleRate;
		mma_deviceConfig.dataCallback      = data_callback;
		mma_deviceConfig.pUserData         = &mma_decoder;


		 if (ma_device_init(NULL, &mma_deviceConfig, &mma_device) != MA_SUCCESS) {
			printf("Failed to open playback device.\n");
			ma_decoder_uninit(&mma_decoder);
			return -3;
		}

	/*	if (ma_device_start(&mma_device) != MA_SUCCESS) {
			printf("Failed to start playback device.\n");
			ma_device_uninit(&mma_device);
			ma_decoder_uninit(&mma_decoder);
			return -4;
		}*/

		return 0;
	}



	void initGeom(const std::string& resourceDirectory, std::string map)
	{

		//EXAMPLE set up to read one shape from one obj file - convert to read several
		// Initialize houseMesh
		// Load geometry
 		// Some obj files contain material information.We'll ignore them for this assignment.
 		vector<tinyobj::shape_t> TOshapes;
 		vector<tinyobj::material_t> objMaterials;
 		string errStr;



		importModelsFrom((resourceDirectory + "/models"), "");

		mainScene.lightLocation = vec3(config["map_values"]["light_location"]["x"].GetFloat(), config["map_values"]["light_location"]["y"].GetFloat(), config["map_values"]["light_location"]["z"].GetFloat());

		int bound = MAP_SIZE;
		shared_ptr<BLZEntity> floor;

		auto minimap_border = BLZEntity::newEntity(meshes["cube.obj"], &mainScene);
		auto staticComponent = c_minimap_static_t{};
		minimap_border->addComponent<c_minimap_static_t>(&mainScene, staticComponent);

		//Todo call the resize function
		auto minimapRenderable = minimap_border->getComponent<c_render_t>();
			minimapRenderable->material = materials["flat_gray"];
			minimapRenderable->tex = textures["minimap.png"];
			minimapRenderable->program = minimapProg;
		auto minimapLocation = minimap_border->getComponent<c_location_t>();
			minimapLocation->size = vec3(bound * 1.3, 0.6, bound * 1.3);
			minimapLocation->position = vec3(0, 100, 0);

		floor = BLZEntity::newEntity(meshes["cube.obj"], &mainScene);
		// Todo call the resize function
		auto floorRenderableComponent = floor->getComponent<c_render_t>();
			floorRenderableComponent->material = materials["flat_gray"];
			floorRenderableComponent->tex = textures["asphalt3.jpg"];
		auto floorLocationComponent = floor->getComponent<c_location_t>();
			floorLocationComponent->size = vec3(bound, 0.6, bound);
			floorLocationComponent->position = vec3(0, -0.3, 0);

		mainScene.skybox = skyboxes["neutral"];

		// contains character info
		shared_ptr<BLZEntity> player = BLZEntity::newEntity(meshes["cube.obj"], &mainScene);
			auto playerComponent = c_player_t{};
			playerComponent.luck = 0; 
			player->addComponent<c_player_t>(&mainScene, playerComponent);
		player->addComponent<c_physics_t>(&mainScene, c_physics_t{});
		resize(player, vec3(config["player_size"]["x"].GetFloat(), config["player_size"]["y"].GetFloat(), config["player_size"]["z"].GetFloat()), &mainScene);
		move(player, vec3(config["player_position"]["x"].GetFloat(), config["player_position"]["y"].GetFloat(), config["player_position"]["z"].GetFloat()), &mainScene);
	}



	

	void render() {
		// Get current frame buffer size.
        int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		glViewport(0, 0, width, height);
		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		RenderDepthMap(mainScene.getEntitiesWithComponents({COMPONENT_LOCATION, COMPONENT_MODEL, COMPONENT_RENDERABLE}), &mainScene);
		
		
		if (width == 0 || height == 0) {
			return;
		}

		float aspect = width/(float)height;
		// Create the matrix stacks - please leave these alone for now
		auto Projection = make_shared<MatrixStack>();
		auto View = make_shared<MatrixStack>();
		auto Model = make_shared<MatrixStack>();

        Projection->pushMatrix();
            Projection->perspective(45.0f, aspect, 0.01f, 10000.0f);
        View->pushMatrix();
            View->loadIdentity();
            View->lookAt(globalCamera->eye, globalCamera->lookAt, globalCamera->up);

		auto player = getPlayer(&mainScene)->getComponent<c_player_t>();

		glBindFramebuffer(GL_FRAMEBUFFER, flashFrameBuf);
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		renderParticles(&mainScene, width, height);
		RenderScene(prog, &mainScene, width, height);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		tintProg->bind();
		    glActiveTexture(GL_TEXTURE0);
    		glBindTexture(GL_TEXTURE_2D, flashTexBuf);
			glUniform1i(tintProg->getUniform("screen"), 0);
			glUniform1f(tintProg->getUniform("uTime"), (float) glfwGetTime());
			glUniform1f(tintProg->getUniform("health"), (float) player->health);
			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *) 0);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glDisableVertexAttribArray(0);
		tintProg->unbind();


		updateGUI(&mainScene, width, height);	
		//set viewport for minimap
		
	}

    void CheckGameOutcome(BScene *scene){

	}


	double lastUpdate;

	void update() {
		double timeDelta = glfwGetTime() - lastUpdate;
		lastUpdate += timeDelta;
		// Use a rolling average technique from networking queues

		DelayedDespawn(&mainScene, timeDelta); // Should be first
		UpdateScenePhysics(&mainScene,(float) timeDelta);
		
		CheckPhysicsCollisions(&mainScene, timeDelta); // After UpdateScenePhysics
		SpawnObjects(&mainScene, timeDelta);
		UpdateObjects(&mainScene, timeDelta);
		updateParticles(timeDelta);
		updateGameCameraLook(timeDelta, windowManager->getHandle());


		// Update camera location
		{
			vec3 forward = globalCamera->lookAt - globalCamera->eye;
			forward.y = 0;
			forward = normalize(forward);
			vec3 right = cross(forward, globalCamera->up);
			right.y = 0;
			right = normalize(right);
			auto velocity = ((right * playerMotion.x)) * 43.0f;
			globalCamera->eye += velocity * (float) timeDelta;
			globalCamera->lookAt += velocity * (float) timeDelta;

			auto player = getPlayer(&mainScene);
			player->getComponent<c_physics_t>()->velocity = velocity;
		}

        CheckGameOutcome(&mainScene);
	}

	void drawLoadingMenu() {
	}

};



int main(int argc, char *argv[])
{
	srand(time(NULL));

	// Where the resources are loaded from
	std::string resourceDir = "resources";
	
	if (argc >= 2)
	{
		resourceDir = argv[1];
	}

	loadConfig(resourceDir);
	Application *application = new Application();

	// Your main will always include a similar set up to establish your window
	// and GL context, etc.

	WindowManager *windowManager = new WindowManager();
	windowManager->init(640, 480);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	// Clear the viewport while we are loading in assets
	glViewport(0, 0, 640, 480);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glfwSwapBuffers(windowManager->getHandle());
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// This is the code that will likely change program to program as you
	// may need to initialize or set up different data and state

	application->initUI(resourceDir);
	application->init(resourceDir);
	application->initMaterials(resourceDir);
	application->initScene();
	application->initGeom(resourceDir, config["map_values"]["map_name"].GetString());
	application->initAudio();
	initGUI(windowManager->getHandle());
	initShadow(); // OpenGL calls must be on main thread
	setupParticleSystem(); // OpenGL calls must be on main thread
	application->lastUpdate = glfwGetTime();

	// Loop until the user closes the window.
	while (! glfwWindowShouldClose(windowManager->getHandle()))
	{
		// Update scene.
		application->update();
		// Render
		application->render();

		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	// Quit program.
	windowManager->shutdown();
	return 0;
}
