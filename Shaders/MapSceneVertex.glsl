#version 330 core
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

in vec3 position;
in vec4 colour;

out Vertex {
    vec4 colour;
    vec3 worldPos;
} OUT;

void main(void) {
    OUT.colour = colour;
    OUT.worldPos = modelMatrix * vec4(position, 1);
    gl_Position = (projMatrix * viewMatrix) * OUT.worldPos;
}