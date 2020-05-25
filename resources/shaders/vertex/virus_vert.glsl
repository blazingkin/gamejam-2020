#version  330 core
layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vertNor;
layout(location = 2) in vec2 vertTex;
layout(location = 3) in vec4 rotationQuat;
layout(location = 4) in mat4 M;
uniform mat4 P;
uniform mat4 V;
uniform vec3 lightPos;
uniform vec3 viewVector;


out vec2 vTexCoord;
out vec3 fragNor;
out vec3 lightDir;
out vec3 viewVectorModel;

void main() {

  vec4 vPosition;

  /* Calculate the model rotation based on the quaternion */
  vec3 temp = cross(rotationQuat.xyz, vertPos.xyz) + rotationQuat.w * vertPos.xyz;
  vec3 rotated = vertPos.xyz + 2.0 * cross(rotationQuat.xyz, temp);

  /* First model transforms */

  gl_Position = P * V * M * vec4(rotated, 1.0);
  vec3 worldVPos = (M * vec4(rotated, 1.0)).xyz;

  vec3 tempNor = cross(rotationQuat.xyz, vertNor.xyz) + rotationQuat.w * vertNor.xyz;
  vec3 rotatedNor = vertNor.xyz + 2.0 * cross(rotationQuat.xyz, tempNor);

  fragNor = (M * vec4(rotatedNor, 0.0)).xyz;
  vTexCoord = vertTex;

  /* Calculate light and view vectors used for lighting */
  viewVectorModel = normalize(viewVector - worldVPos);
	lightDir = normalize(lightPos - worldVPos);
}
