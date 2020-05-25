#version 330 core
uniform sampler2D Texture0;
uniform sampler2D normalMap;
uniform sampler2D shadowDepth;
uniform vec3 MatDif;
uniform vec3 MatAmb;
uniform vec3 MatSpec;
uniform vec3 lightColor;
uniform mat4 M;
uniform int textureEnabled;
uniform float shine;
in vec3 fragNor;
in vec3 lightDir;
in vec3 matColor;
in vec3 viewVectorModel;
out vec4 color;
in vec2 vTexCoord;
in vec4 fPosLS;

float TestShadow(vec4 LSfPos) {
  float bias = 0.005f;
	//1: shift the coordinates from -1, 1 to 0 ,1
  vec3 shifted = (LSfPos.xyz + vec3(1, 1, 1)) * 0.5;
	//2: read off the stored depth (.) from the ShadowDepth, using the shifted.xy 
  vec4 Ld = texture(shadowDepth, shifted.xy);
	//3: compare to the current depth (.z) of the projected depth
  if (Ld.r + bias  < shifted.z) {
    return 1.0;
  }
	//4: return 1 if the point is shadowed

	return 0.0;
}

void main() {
  	vec4 texColor0 = texture(Texture0, vTexCoord);
	vec3 normalMapDir = texture(normalMap, vTexCoord).xyz;

	vec3 lightDir = normalize(lightDir);
	vec3 viewVector = normalize(viewVectorModel);
	vec3 fragNor = normalize(fragNor);

	float lightIntesity = max(dot(lightDir, fragNor), 0);
	vec3 diffColor, ambientColor;
	if (textureEnabled == 1) { 
		diffColor = lightIntesity * texColor0.xyz * lightColor;
		ambientColor =  (MatAmb * lightColor * texColor0.xyz);
	} else {
		diffColor = lightIntesity * MatDif * lightColor;
		ambientColor =  (MatAmb * lightColor);
	}

	if (texColor0.w < 0.1) {
		discard;
	}

	vec3 halfV = (lightDir + viewVector) / length(lightDir + viewVector);
	vec3 colorSpec = MatSpec * (pow(max(dot(fragNor, halfV), 0), shine)) * lightColor;
	
	float Shade = TestShadow(fPosLS);
	
	color = vec4(ambientColor + ((1 - Shade) * (diffColor + colorSpec)), texColor0.w);
}

