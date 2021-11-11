#version 330 core
#define MAX_LIGHTS 64

uniform sampler2D diffuseTex;
uniform sampler2D bumpTex;
uniform sampler2D shadowTex;
uniform sampler2D shadowTex2;
uniform sampler2D shadowTex3;

uniform vec3 cameraPos;
uniform vec4 lightColour[MAX_LIGHTS];
uniform vec4 specLightColour[MAX_LIGHTS];
uniform vec3 lightPos[MAX_LIGHTS];
uniform float numberOfLights;
uniform int lightType[MAX_LIGHTS];
uniform int useShadows[MAX_LIGHTS];

in Vertex {
    vec3 colour;
    vec2 texCoord;
    vec3 normal;
    vec3 tangent;
    vec3 binormal;
    vec3 worldPos;
    vec4 shadowProj[3];
} IN;

vec4 ambientLight = vec4(1,1,1,1);

out vec4 fragColour;

void main(void){
    fragColour = vec4(0,0,0,0);
    vec4 diffuse = texture(diffuseTex, IN.texCoord);

    for(int i = 0; i < numberOfLights; i++){

        vec4 currentColour = vec4(0,0,0,0);
        float attenuation = 1.0;
        vec3 incident;
        if(lightType[i] == 0){ // POINT
            incident = normalize(lightPos[i] - IN.worldPos);
            float distance = length(lightPos[i] - IN.worldPos);
            attenuation = 1.0 / pow(distance, 2);
            attenuation *= (50000 / 20);
        } else {
            incident = normalize(lightPos[i]);
        }

        vec3 viewDir = normalize(cameraPos - IN.worldPos);
        vec3 halfDir = normalize (incident + viewDir);

        float lambert = max(dot(incident, IN.normal), 0.0);

        float specFactor = clamp(dot(halfDir, IN.normal), 0.0, 1.0);
        specFactor = pow(specFactor, 60.0);
        float shadow = 1.0;
        if(useShadows[i] == 1){
            vec3 shadowNDC = IN.shadowProj[i].xyz / IN.shadowProj[i].w;

            if(abs(shadowNDC.x) < 1.0 && abs(shadowNDC.y) < 1.0 && abs(shadowNDC.z) < 1.0) {
                vec3 biasCoord = shadowNDC * 0.5 + 0.5;
                float shadowZ;
                if(i == 0)
                    shadowZ = texture(shadowTex, biasCoord.xy).x;
                if(i == 1)
                    shadowZ = texture(shadowTex2, biasCoord.xy).x;
                if(i == 2)
                    shadowZ = texture(shadowTex3, biasCoord.xy).x;
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