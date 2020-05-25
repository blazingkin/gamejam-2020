#version  330 core

uniform mat4 P;
uniform mat4 V;
uniform float uTime;

layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vertNor;
layout(location = 2) in vec2 vertTex;
layout(location = 3) in vec4 offset;
layout(location = 4) in vec4 lifetime;
layout(location = 5) in vec4 velocity;
layout(location = 6) in vec4 aAnimationFrame;

out float vLifetime;
out vec2 vTextureCoords;
out float animationFrame;

void main(void){
    // resets particle at the end of lifetime
    float time = mod(uTime, lifetime.x);

    vec3 CameraRightWorldspace = vec3(V[0][0], V[1][0], V[2][0]);
    vec3 CameraUpWorldspace = vec3(V[0][1], V[1][1], V[2][1]);
    float size = 4;
    vec3 vertexPosition_worldspace = (offset.xyz + (time * velocity.xyz)) \
    + CameraRightWorldspace * vertPos.x * size
    + CameraUpWorldspace * vertPos.y * size;

    // particles start at position and decreases as it ages
    vec4 position = vec4(vertexPosition_worldspace, 1.0);
    vLifetime = 1.3 - (time/lifetime.x);
    vLifetime = clamp(vLifetime, 0.0, 1.0);


    gl_Position = P * V * position;
    vTextureCoords = vertTex;
    animationFrame = aAnimationFrame.x;
}