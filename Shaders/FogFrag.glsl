#version 330 core

uniform sampler2D sceneTex;
uniform sampler2D depthTex;

uniform int isVertical;

vec4 sky = vec4(0,0,0,1.0);

in Vertex {
    vec2 texCoord;
    smooth vec4 colour;
} IN;

out vec4 fragColour;

void main(void){
    fragColour = (texture(sceneTex, IN.texCoord) * (IN.colour.rrra / 0.75));
}