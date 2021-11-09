#version 330 core

in vec3 position;
in vec4 colour;
in vec2 texCoord;

out Vertex {
	smooth vec4 colour;
    vec2 P;
} OUT;

void main(void){
	gl_Position		= vec4(position, 1.0);
	OUT.colour		= colour;
    OUT.P = texCoord;
}