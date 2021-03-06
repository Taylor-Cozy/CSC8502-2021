#version 330 core

uniform sampler2D diffuseTex;
uniform samplerCube cubeTex;
uniform samplerCube mtnTex;
uniform sampler2D bumpTex;
uniform sampler2D heightmapTex;
uniform vec3 cameraPos;
uniform float time;
uniform float sunTime;

in Vertex {
    vec4 colour;
    vec2 texCoord;
    vec3 normal;
    vec3 tangent;
    vec3 binormal;
    vec3 worldPos;
} IN;

out vec4 fragColour;

void main(void){
    // TODO use sin(time) to affect the mix colour
    fragColour = vec4(0.0,0.41,0.58,0.7);

    mat3 TBN = mat3(normalize(IN.tangent), normalize(IN.binormal), normalize(IN.normal));

    vec3 bumpNormal = texture(bumpTex, IN.texCoord).rgb;
    bumpNormal = normalize(TBN * normalize(bumpNormal * 2.0 - 1.0));

    vec4 diffuse = texture(diffuseTex, IN.texCoord);
    vec3 viewDir = normalize(cameraPos - IN.worldPos);

    vec3 reflectDir = reflect(-viewDir, normalize(bumpNormal));
    vec4 reflectTex1 = texture(mtnTex, reflectDir);
    vec4 reflectTex2 = texture(cubeTex, reflectDir);

    fragColour *= mix(reflectTex1, reflectTex2, sunTime) + (diffuse * 0.25);
}