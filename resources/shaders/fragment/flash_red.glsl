#version 330 core

in vec2 texCoord;
out vec4 color;
uniform sampler2D screen;
uniform float health;
uniform float uTime;

/* just pass through the texture color we will add to this next lab */
void main(){
   vec3 tColor = texture( screen, texCoord ).rgb;
   vec3 screenTint = vec3(0.75, 0.0, 0.0);
   float healthInfluencer = clamp((1 / health) * sin(15 * (1 / health) * uTime),0.0, 0.6);
   if (health > 3) {
       screenTint = tColor;
       healthInfluencer = 0;
   }
   color = vec4(((1 - healthInfluencer) * tColor) + (healthInfluencer * screenTint), 1.0);
}