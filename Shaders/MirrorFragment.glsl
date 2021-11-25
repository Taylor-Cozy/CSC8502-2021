#version 330 core

uniform sampler2D diffuseTex;
uniform samplerCube cubeTex;
uniform samplerCube skyTex;
uniform vec3 cameraPos;
uniform float sunTime;

in Vertex {
    vec3 normal;
    vec3 worldPos;
} IN;

out vec4 fragColour;

void main(void){
    fragColour = vec4(1.0,1.0,1.0,1.0);

    vec3 viewDir = normalize(cameraPos - IN.worldPos);

    vec3 reflectDir = reflect(-viewDir, normalize(IN.normal));
    vec4 reflectTex1 = texture(cubeTex, reflectDir);
    vec4 reflectTex2 = texture(skyTex, reflectDir);

    fragColour *= mix(reflectTex2, reflectTex1, sunTime);
}