#version 330 core

uniform sampler2D diffuseTex;

in Vertex {
    vec2 texCoord;
    vec4 jointWeights;
} IN;

out vec4 fragColour;

void main(void) {
    fragColour = texture(diffuseTex, IN.texCoord);

    // if(IN.jointWeights.yz == vec2(0,0)){
    //     discard;
    // }
}