R"(
#version 130

attribute vec3 position;
attribute vec2 textCoord;

varying vec2 textCoord0;

uniform mat4 transform;
uniform vec3 blockTranslation;

void main() {
	gl_Position = transform * vec4(position + blockTranslation, 1.0);
	textCoord0 = textCoord;
}
)"
