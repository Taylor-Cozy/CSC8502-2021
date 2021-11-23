#version 330 core

uniform sampler2D sceneTex;

uniform int isVertical;

in Vertex {
    vec2 texCoord;
} IN;

out vec4 fragColour[3];

void main(void){
    fragColour[2] = texture(sceneTex, IN.texCoord);

    float brightness = dot(fragColour[2].rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 0.9){
        fragColour[0] = vec4(fragColour[2].rgb, 1.0);
    } else {
        fragColour[0] = vec4(0,0,0,0);
        //discard;
    }

}