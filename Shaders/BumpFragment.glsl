#version 330 core
#define MAX_LIGHTS 64

uniform sampler2D diffuseTex;
uniform sampler2D bumpTex;

uniform vec3 cameraPos;
uniform vec4 lightColour[MAX_LIGHTS];
uniform vec4 specLightColour[MAX_LIGHTS];
uniform vec3 lightPos[MAX_LIGHTS];
uniform float numberOfLights;

in Vertex {
    vec4 colour;
    vec2 texCoord;
    vec3 normal;
    vec3 tangent;
    vec3 binormal;
    vec3 worldPos;
    float visibility;
} IN;

vec4 sky = vec4(0.2,0.2,0.2,1.0);
vec4 ambientLight = vec4(1,1,1,1);
out vec4 fragColour;

void main(void) {
    fragColour = vec4(0,0,0,0);
    vec4 diffuse = texture(diffuseTex, IN.texCoord);

    for(int i = 0; i < numberOfLights; i++){

        vec4 currentColour = vec4(0,0,0,0);
        vec3 incident = normalize(lightPos[i] - IN.worldPos);
        vec3 viewDir = normalize(cameraPos - IN.worldPos);
        vec3 halfDir = normalize (incident + viewDir);

        mat3 TBN = mat3(normalize(IN.tangent), normalize(IN.binormal), normalize(IN.normal));

        vec3 bumpNormal = texture(bumpTex, IN.texCoord).rgb;
        bumpNormal = normalize(TBN * normalize((bumpNormal * 2.0) - 1.0));
        //bumpNormal = normalize(TBN * normalize((bumpNormal + 1.0) / 2.0));

        float lambert = max(dot(incident, bumpNormal), 0.0);
        float distance = length(lightPos[i] - IN.worldPos);
        // float attenuation = 1.0 - clamp(distance / lightRadius, 0.0, 1.0);
        float attenuation = 1.0 / pow(distance, 2);
        attenuation *= (50000);

        float specFactor = clamp(dot(halfDir, bumpNormal), 0.0, 1.0);
        specFactor = pow(specFactor, 60.0);

        vec3 surface = (diffuse.rgb * lightColour[i].rgb);
        currentColour.rgb = surface * lambert * attenuation;
        currentColour.rgb += (specLightColour[i].rgb * specFactor) * attenuation * 0.33;
        currentColour.a = diffuse.a;

        fragColour += currentColour;
    }
    fragColour += ambientLight * 0.2;
    // FOG
    //fragColour = mix(sky, fragColour, IN.visibility);
}