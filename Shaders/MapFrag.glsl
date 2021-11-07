#version 330 core

uniform sampler2D sceneTex;

uniform int isVertical;

in Vertex {
    vec2 texCoord;
} IN;

out vec4 fragColour;

void main(void){
    fragColour = texture2D(sceneTex, IN.texCoord.xy);
}