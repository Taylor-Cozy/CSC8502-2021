#version 330 core

uniform sampler2D diffuseTex;
uniform samplerCube cubeTex;
uniform sampler2D bumpTex;
uniform sampler2D heightmapTex;
uniform vec3 cameraPos;
uniform float time;

in Vertex {
    vec4 colour;
    vec2 texCoord;
    vec2 seaFoamTexCoord;
    vec3 normal;
    vec3 tangent;
    vec3 binormal;
    vec3 worldPos;
} IN;

out vec4 fragColour;

void main(void){
    fragColour = vec4(1.0,1.0,1.0,1.0);

    vec3 viewDir = normalize(cameraPos - IN.worldPos);

    vec3 reflectDir = reflect(-viewDir, normalize(IN.normal));
    vec4 reflectTex = texture(cubeTex, reflectDir);

    fragColour *= reflectTex;

}