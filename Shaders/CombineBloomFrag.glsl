#version 330 core

uniform sampler2D sceneTex;
uniform sampler2D bloomBlur;

in Vertex {
    vec2 texCoord;
} IN;

out vec4 fragColour;

void main(void){
    //fragColour = texture(sceneTex, IN.texCoord);
    vec3 diffuse = texture(sceneTex, IN.texCoord).rgb;
    vec3 bloom = texture(bloomBlur, IN.texCoord).rgb;
    if(bloom.r == 0 && bloom.g == 0 && bloom.b == 0){
        discard;
    }
    fragColour = vec4(diffuse + bloom, 1.0);
}