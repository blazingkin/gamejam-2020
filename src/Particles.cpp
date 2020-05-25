#include <iostream>
#include "Particles.h"
#include "GLSL.h"
#include "MatrixStack.h"
#include "Program.h"
#include "Texture.h"
#include "Shape.h"


shared_ptr<Program> particleProg;
particle_state_t state;
shared_ptr<Shape> square;

void setupParticleSystem() {
    particleProg = make_shared<Program>();
    particleProg->enableInstanceRendering();
    const vector<string> shaders = {"particle_vert.glsl", "particle_frag.glsl"};
    particleProg->init(shaders);
    particleProg->addUniform("P");
    particleProg->addUniform("V");
    particleProg->addUniform("uTime");
    particleProg->addUniform("Texture0");

    particleProg->addAttribute("vertPos");
    particleProg->addAttribute("vertNor");
    particleProg->addAttribute("vertTex");
    particleProg->addAttribute("offset");
    particleProg->addAttribute("lifetime");
    particleProg->addAttribute("velocity");
    particleProg->addAttribute("aAnimationFrame");
    

    square = make_shared<Shape>();
    tinyobj::shape_t square_shape;
    // top left
    square_shape.mesh.positions.push_back(-1.0);
    square_shape.mesh.positions.push_back(1.0);
    square_shape.mesh.positions.push_back(0);
    square_shape.mesh.texcoords.push_back(0);
    square_shape.mesh.texcoords.push_back(0);

    // top right
    square_shape.mesh.positions.push_back(1.0);
    square_shape.mesh.positions.push_back(1.0);
    square_shape.mesh.positions.push_back(0);
    square_shape.mesh.texcoords.push_back(1);
    square_shape.mesh.texcoords.push_back(0);

    // bottom left
    square_shape.mesh.positions.push_back(-1.0);
    square_shape.mesh.positions.push_back(-1.0);
    square_shape.mesh.positions.push_back(0);
    square_shape.mesh.texcoords.push_back(0);
    square_shape.mesh.texcoords.push_back(1);

    // bottom right
    square_shape.mesh.positions.push_back(1.0);
    square_shape.mesh.positions.push_back(-1.0);
    square_shape.mesh.positions.push_back(0);
    square_shape.mesh.texcoords.push_back(1);
    square_shape.mesh.texcoords.push_back(1);

    square_shape.mesh.indices.push_back(0);
    square_shape.mesh.indices.push_back(2);
    square_shape.mesh.indices.push_back(3);
    //  |\ 
    //  | \ 
    //  |__\ 
    
    square_shape.mesh.indices.push_back(0);
    square_shape.mesh.indices.push_back(1);
    square_shape.mesh.indices.push_back(3);
    // ___
    // \  |
    //  \ |
    //   \| 
    square->createShape(square_shape);

    
}
static std::mutex particleLock;
void updateParticles(float deltaTime) {
    particleLock.lock();
    particle_state_t newState;
    for (size_t i = 0; i < state.startingPosition.size(); i++) {
        float lifeTime = state.life[i] - deltaTime;
        if (lifeTime <= 0.0f) {
            continue;
        }
        newState.startingPosition.push_back(state.startingPosition[i]);
        newState.velocity.push_back(state.velocity[i]);
        newState.animationFrame.push_back(state.animationFrame[i]);
        newState.Lifetime.push_back(state.Lifetime[i]);
        newState.life.push_back(lifeTime);
    }
    state = newState;
    particleLock.unlock();
}

void addParticle(vec3 initialPosition, vec3 velocity, float atlasPosition, float lifetime) {
    
    particleLock.lock();
    state.startingPosition.push_back(vec4(initialPosition, 0.0f));
    state.velocity.push_back(vec4(velocity, 0.0f));
    state.Lifetime.push_back(vec4(lifetime, 0.0f, 0.0f, 0.0f));
    state.animationFrame.push_back(vec4(atlasPosition, 0.0f, 0.0f, 0.0f));
    state.life.push_back(lifetime);
    particleLock.unlock();
}

void renderParticles(BScene *scene, int width, int height) {
    float aspect = width / (float) height;
    particleProg->bind();
    glUniformMatrix4fv(particleProg->getUniform("P"), 1, GL_FALSE, value_ptr(glm::perspective(45.0f, aspect, 0.01f, 500.0f)));
    glUniformMatrix4fv(particleProg->getUniform("V"), 1, GL_FALSE, value_ptr(glm::lookAt(globalCamera->eye, globalCamera->lookAt, globalCamera->up)));
    glUniform1f(particleProg->getUniform("uTime"), (float) glfwGetTime());
    particleProg->setTexture(textures["particleAtlas.png"]);

    instance_render_data_t renderData;
    renderData.attributes["offset"] = make_shared<vector<vec4>>(state.startingPosition);
    renderData.attributes["lifetime"] = make_shared<vector<vec4>>(state.Lifetime);
    renderData.attributes["velocity"] = make_shared<vector<vec4>>(state.velocity);
    renderData.attributes["aAnimationFrame"] = make_shared<vector<vec4>>(state.animationFrame);

    if (state.startingPosition.size() > 0) {
        for (auto shp : *meshes["square.obj"]) {
            shp.drawInstances(particleProg, state.startingPosition.size(), &renderData);
        }
        
    }
    
    particleProg->unbind();
}