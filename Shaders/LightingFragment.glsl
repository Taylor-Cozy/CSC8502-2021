#version 330 core
#define MAX_LIGHTS 64

uniform sampler2D diffuseTex;
uniform sampler2D diffuseBump;
uniform sampler2D secondaryTex;
uniform sampler2D secondaryBump;
uniform sampler2D thirdTex;
uniform sampler2D thirdBump;
uniform sampler2D shadowTex;
uniform samplerCube shadowCube;

uniform int useTexture;

uniform vec3 cameraPos;
uniform vec4 lightColour[MAX_LIGHTS];
uniform vec4 specLightColour[MAX_LIGHTS];
uniform vec3 lightPos[MAX_LIGHTS];
uniform float numberOfLights;
uniform int lightType[MAX_LIGHTS];
uniform int useShadows[MAX_LIGHTS];
uniform mat4 shadowMatrices[MAX_LIGHTS];
uniform vec2 shadowTexOffsets[MAX_LIGHTS];

uniform vec4 nodeColour;

in Vertex {
    vec4 colour;
    vec2 texCoord;
    vec3 normal;
    vec3 tangent;
    vec3 binormal;
    vec4 worldPos;
} IN;

vec4 sky = vec4(0.2,0.2,0.2,1.0);
vec4 ambientLight = vec4(1,1,1,1);
out vec4 fragColour;

void main(void) {
    fragColour = vec4(0,0,0,0);
    
    vec4 diffuse;
    float wetness = 1.0;
    
    if(useTexture > 0){

        if(abs(IN.normal.x) > 0.25 || abs(IN.normal.z) > 0.25){
            diffuse = mix(texture(diffuseTex, IN.texCoord),texture(thirdTex, IN.texCoord), (max(abs(IN.normal.x) , abs(IN.normal.z)) - 0.25) * 4) ;
        } else {
            diffuse = texture(diffuseTex, IN.texCoord);
        }

        if(IN.worldPos.y < 160){
            diffuse = texture(secondaryTex, IN.texCoord);
        } else if(IN.worldPos.y < 190) {
            diffuse = mix(diffuse,texture(secondaryTex, IN.texCoord), (190 - IN.worldPos.y) / 30.0);
            wetness = ((190 - IN.worldPos.y) / 30.0) + 0.2;
        } else {

            wetness = 0.2;
        }
    } else {
        diffuse = nodeColour;
    }

    if(wetness > 1.0){
        wetness = 1.0;
    }

    for(int i = 0; i < numberOfLights; i++){
        float distance = distance(IN.worldPos.xyz, lightPos[i]);
        if( distance < 500 || lightType[i] == 1){
            vec4 currentColour = vec4(0,0,0,0);
            float attenuation = 1.0;
            vec3 incident;

            if(lightType[i] == 0){ // POINT
                incident = normalize(lightPos[i] - IN.worldPos.xyz);
                attenuation = 1.0 / pow(distance, 2);
                attenuation *= (10000);
                if(useShadows[i] == 1.0){
                    attenuation *= 100;
                }
            } else {
                incident = normalize(lightPos[i]);
            }

            vec3 viewDir = normalize(cameraPos - IN.worldPos.xyz);
            vec3 halfDir = normalize (incident + viewDir);

            mat3 TBN = mat3(normalize(IN.tangent), normalize(IN.binormal), normalize(IN.normal));
            
            vec3 bumpNormal;

            if(abs(IN.normal.x) > 0.25 || abs(IN.normal.z) > 0.25){
                bumpNormal = mix(texture(diffuseBump, IN.texCoord),texture(thirdBump, IN.texCoord), (max(abs(IN.normal.x) , abs(IN.normal.z)) - 0.25) * 4).rgb;
            } else {
                bumpNormal = texture(diffuseBump, IN.texCoord).rgb;
            }

            if(IN.worldPos.y < 160){
                bumpNormal = texture(secondaryBump, IN.texCoord).rgb;
            } else if(IN.worldPos.y < 190) {
                bumpNormal = mix(bumpNormal,texture(secondaryBump, IN.texCoord).rgb, (190 - IN.worldPos.y) / 30.0);
            }

            bumpNormal = normalize(TBN * normalize((bumpNormal * 2.0) - 1.0));
            //bumpNormal = normalize(TBN * normalize((bumpNormal + 1.0) / 2.0));

            float lambert;
            float specFactor;

            float distance = length(lightPos[i] - IN.worldPos.xyz);

            if(useTexture > 0){
                lambert = max(dot(incident, bumpNormal), 0.0);
                specFactor = clamp(dot(halfDir, bumpNormal), 0.0, 1.0);
            } else {
                lambert = max(dot(incident, IN.normal), 0.0);
                specFactor = clamp(dot(halfDir, IN.normal), 0.0, 1.0);
            }
            
            specFactor = pow(specFactor, 60.0);

            float shadow = 1.0;
            if(useShadows[i] == 1){
                vec3 viewDir = normalize(lightPos[i] - IN.worldPos.xyz);
                vec4 pushVal = vec4(IN.normal, 0) * dot(viewDir, IN.normal);
                vec4 shadowProj = shadowMatrices[i] * (IN.worldPos + pushVal);

                vec3 shadowNDC = shadowProj.xyz / shadowProj.w;
                
                if(abs(shadowNDC.x) < 1.0 && abs(shadowNDC.y) < 1.0 && abs(shadowNDC.z) < 1.0) {
                    vec3 biasCoord = (shadowNDC) * 0.5 + 0.5;
                    float shadowZ = texture(shadowTex, vec2((biasCoord.x / 4.0) + shadowTexOffsets[i].x, (biasCoord.y / 4.0) + shadowTexOffsets[i].y)).x;
                    if(shadowZ < biasCoord.z) {
                        shadow = 0.0;
                    }
                } else {
                    shadow = 0.0;
                }
            } 



            vec3 surface = (diffuse.rgb * lightColour[i].rgb);
            currentColour.rgb = surface * lambert * attenuation;
            currentColour.rgb += (specLightColour[i].rgb * specFactor * wetness) * attenuation * 0.33;
            currentColour *= shadow;
            currentColour.a = diffuse.a;
            fragColour += currentColour;
        }
        
    }
    fragColour += ambientLight * 0.1;
}