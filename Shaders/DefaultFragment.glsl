#version 330 core

uniform sampler2D diffuseTex;
uniform vec4 nodeColour;
uniform int useTexture;

in Vertex {
    vec4 colour;
    vec2 texCoord;
} IN;

out vec4 fragColour;

void main(void){
    fragColour = nodeColour;
    if(useTexture == 1){
        fragColour = texture(diffuseTex, IN.texCoord);
    }
}