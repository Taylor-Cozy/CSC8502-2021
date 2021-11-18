#version 330 core

uniform sampler2D diffuseTex;
uniform sampler2D secondaryTex;
uniform sampler2D thirdTex;

in Vertex {
    vec2 texCoord;
    vec4 colour;
    vec3 worldPos;
} IN;

out vec4 fragColour;

void main(void){
    if(IN.worldPos.y < 170){
        fragColour = texture(secondaryTex, IN.texCoord);
    } else if(IN.worldPos.y < 200) {
        fragColour = mix(texture(diffuseTex, IN.texCoord),texture(secondaryTex, IN.texCoord), (200 - IN.worldPos.y) / 30.0);
    } else {
        fragColour = texture(diffuseTex, IN.texCoord);
    }
    
}