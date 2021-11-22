#version 330 core
#define MAX_LIGHTS 64

uniform sampler2D diffuseTex;
uniform sampler2D secondaryTex;

uniform vec3 cameraPos;
uniform vec4 lightColour[MAX_LIGHTS];
uniform vec4 specLightColour[MAX_LIGHTS];
uniform vec3 lightPos[MAX_LIGHTS];
uniform float numberOfLights;
uniform int lightType[MAX_LIGHTS];

in Vertex {
    vec4 colour;
    vec2 texCoord;
    vec3 normal;
    vec3 worldPos;
    float visibility;
} IN;

vec4 sky = vec4(0.2,0.2,0.2,1.0);
vec4 ambientLight = vec4(1,1,1,1);
out vec4 fragColour;

void main(void) {
    vec4 diffuse;

    if(IN.worldPos.y < 170){
        diffuse = texture(secondaryTex, IN.texCoord);
    } else if(IN.worldPos.y < 200) {
        diffuse = mix(texture(diffuseTex, IN.texCoord),texture(secondaryTex, IN.texCoord), (200 - IN.worldPos.y) / 30.0);
    } else {
        diffuse = texture(diffuseTex, IN.texCoord);
    }

    for(int i = 0; i < numberOfLights; i++){

        vec4 currentColour = vec4(0,0,0,0);
        float attenuation = 1.0;
        vec3 incident;
        if(lightType[i] == 0){ // POINT
            incident = normalize(lightPos[i] - IN.worldPos);
            float distance = length(lightPos[i] - IN.worldPos);
            attenuation = 1.0 / pow(distance, 2);
            attenuation *= (50000 * 2);
        } else {
            incident = normalize(lightPos[i]);
        }

        vec3 viewDir = normalize(cameraPos - IN.worldPos);
        vec3 halfDir = normalize (incident + viewDir);

        float lambert = max(dot(incident, IN.normal), 0.0);

        float specFactor = clamp(dot(halfDir, IN.normal), 0.0, 1.0);
        specFactor = pow(specFactor, 60.0) * 0.0;

        vec3 surface = (diffuse.rgb * lightColour[i].rgb);
        currentColour.rgb = surface * lambert * attenuation;
        currentColour.rgb += (specLightColour[i].rgb * specFactor) * attenuation * 0.33;
        currentColour.a = diffuse.a;

        fragColour += currentColour;
    }

    fragColour += ambientLight * .1;
    
    // FOG
    
    // if(IN.worldPos.y < 100){
    //     fragColour *= vec4(.76,.69,0.5,1);
    // } else if(IN.worldPos.y < 120){
    //     fragColour *= mix(vec4(0,.5,0,1), vec4(.76,.69,0.5,1), (120 - IN.worldPos.y) / 20.0);
    // }
    // else{
    //     fragColour *= vec4(0,.5,0,1);
    // }

    // if(IN.normal.y < 0.75){
    //     fragColour *= vec4(0.58,.29,0,1);
    // } else {
    //     fragColour *= vec4(0.27, 0.29, 0.1, 1);
    // }

    //fragColour = mix(sky, fragColour, IN.visibility);
}