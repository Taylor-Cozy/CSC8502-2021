#version 330 core
#define MAX_LIGHTS 64

uniform sampler2D diffuseTex;
uniform sampler2D bumpTex;
uniform sampler2D shadowTex;

uniform vec3 cameraPos;
uniform vec4 lightColour[MAX_LIGHTS];
uniform vec4 specLightColour[MAX_LIGHTS];
uniform vec3 lightPos[MAX_LIGHTS];
uniform float numberOfLights;
uniform int lightType[MAX_LIGHTS];
uniform int useShadows[MAX_LIGHTS];
uniform mat4 shadowMatrices[MAX_LIGHTS];

in Vertex {
    vec3 colour;
    vec2 texCoord;
    vec3 normal;
    vec3 tangent;
    vec3 binormal;
    vec4 worldPos;
} IN;

vec4 ambientLight = vec4(1,1,1,1);

out vec4 fragColour;

void main(void){
    fragColour = vec4(0,0,0,0);
    vec4 diffuse = texture(diffuseTex, IN.texCoord);

    float xOffset = 0; float yOffset = 0;

    for(int i = 0; i < numberOfLights; i++){

        vec4 currentColour = vec4(0,0,0,0);
        float attenuation = 1.0;
        vec3 incident;
        if(lightType[i] == 0){ // POINT
            incident = normalize(lightPos[i] - IN.worldPos.xyz);
            float distance = length(lightPos[i] - IN.worldPos.xyz);
            attenuation = 1.0 / pow(distance, 2);
            attenuation *= (50000 / 20);
        } else {
            incident = normalize(lightPos[i]);
        }

        vec3 viewDir = normalize(cameraPos - IN.worldPos.xyz);
        vec3 halfDir = normalize (incident + viewDir);

        float lambert = max(dot(incident, IN.normal), 0.0);

        float specFactor = clamp(dot(halfDir, IN.normal), 0.0, 1.0);
        specFactor = pow(specFactor, 60.0);
        float shadow = 1.0;
        if(useShadows[i] == 1){

            vec3 viewDir = normalize(lightPos[i] - IN.worldPos.xyz);
            vec4 pushVal = vec4(IN.normal, 0) * dot(viewDir, IN.normal);
            vec4 shadowProj = shadowMatrices[i] * (IN.worldPos + pushVal);

            vec3 shadowNDC = shadowProj.xyz / shadowProj.w;

            if(abs(shadowNDC.x) < 1.0 && abs(shadowNDC.y) < 1.0 && abs(shadowNDC.z) < 1.0) {
                vec3 biasCoord = (shadowNDC) * 0.5 + 0.5;
                float shadowZ = texture(shadowTex, vec2((biasCoord.x / 2.0) + xOffset, (biasCoord.y / 2.0) + yOffset)).x;
                xOffset += 0.5;
                if(xOffset == 1.0){
                    xOffset = 0;
                    yOffset += 0.5;
                }
                
                if(shadowZ < biasCoord.z) {
                    shadow = 0.0;
                }
            }
        }


        vec3 surface = (diffuse.rgb * lightColour[i].rgb);
        currentColour.rgb = surface * lambert * attenuation;
        currentColour.rgb += (specLightColour[i].rgb * specFactor) * attenuation * 0.33;
        currentColour *= shadow;
        currentColour.a = diffuse.a;
        fragColour += currentColour;
    }
    fragColour += ambientLight * .1;
    
}
/*

    */