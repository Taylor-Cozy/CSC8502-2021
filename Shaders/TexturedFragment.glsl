#version 330 core

uniform sampler2D diffuseTex;
uniform sampler2D secondTex;

in Vertex {
    vec2 texCoord;
    smooth vec4 colour;
} IN;

out vec4 fragColour;

void main(void) {
    fragColour = texture(secondTex, IN.texCoord) * texture(diffuseTex, IN.texCoord); //texture(diffuseTex, IN.texCoord);
}