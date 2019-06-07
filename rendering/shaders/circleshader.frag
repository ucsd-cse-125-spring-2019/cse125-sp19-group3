#version 330 core

out vec4 finalColor;

uniform vec4 color = vec4(1, 0, 0, 0.2);

void main() {
	finalColor = color;
}