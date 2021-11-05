#version 330 core

uniform sampler2D sceneTex;

uniform int isVertical;

in Vertex {
    vec2 texCoord;
} IN;

out vec4 fragColour;

const float horizontal[9] = float[](1, 0, -1, 2, 0, -2, 1, 0, -1);
const float vertical[9] = float[](1, 2, 1, 0, 0, 0, -1, -2, -1);

void main(void){
    fragColour = vec4(0,0,0,1);
    vec2 delta = vec2(0,0);

    delta.y = dFdy(IN.texCoord).y;
    delta.x = dFdx(IN.texCoord).x;

    for(int i = 0; i < 3; i++){
        for(int j = 0; j < 3; j++){
            vec2 offset;
            offset.x = delta.x * (i-1);
            offset.y = delta.y * (j-1);
            vec4 tmp = texture2D(sceneTex, IN.texCoord.xy + offset).rrra;
            fragColour += tmp * (isVertical != 1 ? vertical[(i * 3) + j] : horizontal[(i * 3) + j]);
        }
    }

    //fragColour = texture2D(sceneTex, IN.texCoord.xy).bbba;
}