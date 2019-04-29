#version 330

precision highp float;
 
struct Light {
	vec3 lPos;
	vec3 lColor;
};
 
uniform sampler2D tex;
uniform mat3 nvp;  // Normal transform (may be adjusted if mvp does nonuniform scaling)
uniform int numLights;
uniform vec3 lPos;
uniform vec3 lColor;
uniform vec3 ambColor;

in vec3 fragNormal;
in vec2 fragTexCoord;
in vec4 fragPos;

out vec4 fragColor;
 
void main(void) {
    vec3 normal = normalize(nvp * fragNormal);
    vec3 lightVec = normalize(lPos - vec3(fragPos));
	
	float dfsBright = dot(normal, lightVec); 
	
	vec4 tempClr = texture(tex, fragTexCoord);
	
    fragColor = vec4((dfsBright * lColor + ambColor) * tempClr.rgb, tempClr.a);
}