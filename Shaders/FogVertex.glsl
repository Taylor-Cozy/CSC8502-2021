#version 330 core

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;
uniform mat4 textureMatrix;

in vec3 position;
in vec2 texCoord;
in vec4 colours;

out Vertex {
    vec2 texCoord;
    smooth vec4 colour;
    float visibility;
} OUT;

void main(void) {
    vec4 distanceFromCam = viewMatrix * modelMatrix * vec4(position, 1.0);
    mat4 mvp = projMatrix * viewMatrix * modelMatrix;
    gl_Position = mvp * vec4(position, 1.0);
    OUT.texCoord = (textureMatrix * vec4(texCoord, 0.0, 1.0)).xy;
    OUT.colour		= colours;
    
    float magDistance = length(distanceFromCam.xyz);
    OUT.visibility = exp(-pow((magDistance * 0.0007), 1.5));
}