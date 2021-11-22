#version 330 core
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

in vec3 position;
in vec4 colour;
in vec3 normal;
in vec4 tangent;
in vec2 texCoord;

out Vertex {
    vec4 colour;
    vec2 texCoord;
    vec3 normal;
    vec3 tangent;
    vec3 binormal;
    vec4 worldPos;
    float visibility;
} OUT;

void main(void) {
    OUT.colour = colour;
    OUT.texCoord = texCoord;

    mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));

    vec3 wNormal = normalize(normalMatrix * normalize(normal));
    vec3 wTangent = normalize(normalMatrix * normalize(tangent.xyz));

    OUT.normal = wNormal;
    OUT.tangent = wTangent;
    OUT.binormal = cross(wTangent, wNormal) * tangent.w;

    OUT.worldPos = (modelMatrix * vec4(position, 1));
    gl_Position = (projMatrix * viewMatrix) * OUT.worldPos;

    // FOG
    vec4 distanceFromCam = viewMatrix * OUT.worldPos;
    float magDistance = length(distanceFromCam.xyz);
    OUT.visibility = exp(-pow((magDistance * 0.0007), 1.5));
}