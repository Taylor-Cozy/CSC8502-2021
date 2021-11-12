#version 330 core
#define MAX_LIGHTS 64

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;
uniform mat4 shadowMatrices[MAX_LIGHTS];
uniform int useShadows[MAX_LIGHTS];
uniform vec3 lightPos[MAX_LIGHTS];
uniform float numberOfLights;

in vec3 position;
in vec3 colour;
in vec3 normal;
in vec4 tangent;
in vec2 texCoord;

out Vertex {
    vec3 colour;
    vec2 texCoord;
    vec3 normal;
    vec3 tangent;
    vec3 binormal;
    vec4 worldPos;
} OUT;

void main(void){
    OUT.colour = colour;
    OUT.texCoord = texCoord;
    
    mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));
    vec3 wNormal = normalize(normalMatrix* normalize(normal));
    vec3 wTangent = normalize(normalMatrix * normalize(tangent.xyz));

    OUT.normal = wNormal;
    OUT.tangent = wTangent;
    OUT.binormal = cross(wNormal, wTangent) * tangent.w;

    OUT.worldPos = (modelMatrix * vec4(position, 1));
    gl_Position = (projMatrix * viewMatrix) * OUT.worldPos;
}