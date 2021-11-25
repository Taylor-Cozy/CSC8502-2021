#version 330 core

uniform samplerCube cubeTex;
uniform samplerCube mtnTex;
uniform float sunTime;

in Vertex {
    vec3 viewDir;
} IN;

out vec4 fragColour;

void main(void){
    vec4 tex1 = texture(cubeTex, normalize(IN.viewDir));
    vec4 tex2 = texture(mtnTex, normalize(IN.viewDir));

    fragColour = mix(tex2, tex1, sunTime);
}