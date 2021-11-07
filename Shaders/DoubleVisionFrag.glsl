#version 330 core

uniform sampler2D sceneTex;

uniform int isVertical;

in Vertex {
    vec2 texCoord;
} IN;

out vec4 fragColour;

void main(void){
    fragColour = vec4(0,0,0,1);
    vec2 delta = vec2(0,0);
    delta = dFdx(IN.texCoord);

    vec2 leftOffset = delta * -10;
    vec2 rightOffset = delta * 10;
    vec4 left = texture2D(sceneTex, IN.texCoord.xy + leftOffset);
    vec4 right = texture2D(sceneTex, IN.texCoord.xy + rightOffset);
    fragColour += ((left * 0.5) + (right * 0.5));
}