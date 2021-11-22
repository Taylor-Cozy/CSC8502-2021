#version 330 core

uniform sampler2D diffuseTex;
uniform sampler2D diffuseBump;
uniform sampler2D secondaryTex;
uniform sampler2D secondaryBump;
uniform sampler2D thirdTex;
uniform sampler2D thirdBump;

uniform int useTexture;

uniform vec4 nodeColour;

in Vertex {
    vec4 colour;
    vec2 texCoord;
    vec3 normal;
    vec3 tangent;
    vec3 binormal;
    vec3 worldPos;
    float visibility;
} IN;

out vec4 fragColour[2];

void main(void){
    mat3 TBN = mat3(normalize(IN.tangent), normalize(IN.binormal), normalize(IN.normal));
    vec3 normal;
    if(IN.worldPos.y < 160){
        fragColour[0] = texture2D(secondaryTex, IN.texCoord);
        normal = texture2D(secondaryBump, IN.texCoord).rgb * 2.0-1.0;
    } else if(IN.worldPos.y < 190) {
        fragColour[0] = mix(texture2D(diffuseTex, IN.texCoord),texture2D(secondaryTex, IN.texCoord), (190 - IN.worldPos.y) / 30.0);
        normal = mix(texture2D(diffuseBump, IN.texCoord), texture2D(secondaryBump, IN.texCoord), (190 - IN.worldPos.y) / 30.0).rgb * 2.0-1.0;
    } else {
        fragColour[0] = texture2D(diffuseTex, IN.texCoord);
        normal = texture2D(diffuseBump, IN.texCoord).rgb * 2.0-1.0;
    }
    normal = normalize(TBN * normalize(normal));
    fragColour[1] = vec4(normal.xyz * 0.5 + 0.5, 1.0);
}