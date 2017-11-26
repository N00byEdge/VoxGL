R"(
#version 420

varying vec2 textCoord0;

uniform sampler2D diffuse;

void main() {
	gl_FragColor = texture2D(diffuse, textCoord0);
}
)"
