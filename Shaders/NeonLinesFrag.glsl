#version 330 core

uniform sampler2D diffuseTex;

in Vertex {
    vec2 texCoord;
    smooth vec4 colour;
} IN;

out vec4 fragColour;

void main(void) {
    float x = mod(IN.texCoord.x, 0.1);
    float y = mod(IN.texCoord.y, 0.1);
    float alpha = 1;

    if(x >= 0.09 || y >= 0.09 || x <= 0.01 || y <= 0.01) {
        if(min(x,y) <= 0.01){
            if(max(x,y) >= 0.09){
                alpha = min((0.1 - max(x,y)), min(x,y));
            } else
                alpha = min(x,y);
        } else {
            alpha = max(x,y);
            alpha = 0.1 - alpha;
        }

        alpha *= 100;
        alpha = pow(alpha, 0.5);
        if(alpha < 0.25){
            alpha = 0;
        }
        fragColour = vec4(1,0.06,0.94,1 - alpha);
    }
    else {
        fragColour = vec4(0,0,0,0);
        //discard;
    }
}