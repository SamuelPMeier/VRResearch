#version 330

layout(location = 0) in vec4 in_Position;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec2 tex_Coord;

uniform mat4 mvp;

out vec3 fragNormal;
out vec2 fragTexCoord;
out vec4 fragPos;

void main(void) {
    gl_Position = fragPos = mvp * in_Position;

	fragNormal = in_Normal;
	fragTexCoord = tex_Coord;
}
