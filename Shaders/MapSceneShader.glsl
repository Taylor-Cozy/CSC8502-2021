#version 330 core

in Vertex {
    vec4 colour;
    vec3 worldPos;
} IN;

out vec4 fragColour;

vec4 sand = vec4(0.857,0.745,0.594,1.0);
vec4 grass = vec4(0.378, 0.426, 0.190, 1.0);
vec4 rock = vec4(0.558, 0.550, 0.476, 1.0);

void main(void) {

    if(IN.worldPos.y < 160){
        fragColour = sand;
    } else if(IN.worldPos.y < 190) {
        diffuse = mix(grass,sand, (190 - IN.worldPos.y) / 30.0);
    } else {
        if(abs(IN.normal.x) > 0.25 || abs(IN.normal.z) > 0.25){
            diffuse = mix(grass,rock, (max(abs(IN.normal.x) , abs(IN.normal.z)) - 0.25) * 4) ;
        } else {
            diffuse = grass;
        }
    }

}