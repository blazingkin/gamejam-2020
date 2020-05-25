#version 330 core

uniform sampler2D Texture0;
uniform float uTime;

in float vLifetime;
in vec2 vTextureCoords;
in float animationFrame;
out vec4 color;

void main(void){
    // reset at the end of lifetime
    float time = mod(uTime, vLifetime);


    // 16 is the number of atlas sections
    float offset = animationFrame;
    float offsetX = floor(mod(offset + 0.2, 4.0))/4.0;
    float offsetY = 0.75 - floor(offset/4.0)/4.0;
    
    vec4 texColor = texture2D(Texture0, vec2((vTextureCoords.x/4.0)+offsetX, (vTextureCoords.y/4) + offsetY));
    if (texColor.w < 0.75) {
        discard;
    }
    color = texColor;
}
