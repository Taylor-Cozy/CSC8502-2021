#version 330 core

in Vertex{
	smooth vec4 colour;
    vec2 P;
} IN;

// vec2 A = vec2(0.5,0.0);
// vec2 B = vec2(1.0,1.0);
// vec2 C = vec2(0.0,1.0);

vec2 A = vec2(0.75,0.5);
vec2 B = vec2(0.25,0.5);
vec2 C = vec2(0.5,1.0);

out vec4 fragColour;

float area(vec2 a, vec2 b, vec2 c){
    return abs(((a.x * (b.y - c.y)) + (b.x * (c.y - a.y)) + (c.x * (a.y - b.y))) / 2.0);
}

void main(void){
    vec2 P = IN.P;

    vec2 halfAngleA = normalize((B-A) + (C-A));
    vec2 halfAngleB = normalize((A-B) + (C-B));
    vec2 halfAngleC = normalize((A-C) + (B-C));

    // A += halfAngleA * 0.1;
    // B += halfAngleB * 0.1;
    // C += halfAngleC * 0.1;

    float totalArea = area(A,B,C);

    float areaPAB = area(P, A, B);
    float areaPBC = area(P, B, C);
    float areaPCA = area(P, C, A);



    if(areaPAB + areaPCA + areaPBC - totalArea < 0.001){
        discard;
    } else {
        fragColour	= IN.colour;
    }
}