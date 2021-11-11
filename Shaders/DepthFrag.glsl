#version 330 core

uniform sampler2D sceneTex;

in Vertex {
    vec2 texCoord;
} IN;

out vec4 fragColour;

void main(void){
    float x = texture2D(sceneTex, IN.texCoord).x;
    x = (2.0 * 1.0) / (10000.0 + 1.0 - x * (10000.0 - 1.0));

    fragColour = vec4(x,x,x,1);

}