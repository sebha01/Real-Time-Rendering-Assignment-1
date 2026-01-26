#version 330

uniform sampler2D texture0;
int halfMultiplier = 1;//scale of bluring
in vec2 texCoord;

layout (location=0) out vec4 fragColour;



void main(void) {
    vec2 textureSize = textureSize(texture0,0);//get size of texture

    float texelSizeS = 1.0 / textureSize.s; //get size of a texel (U)
    float texelSizeT = 1.0 / textureSize.t; //get size of a texel (V)
    vec4 texColour=vec4(0);
    int count=0;
    
    //Sample 9 texels around texCoord and add them together
    for (int i=-1;i<=1;i++)
        for (int j=-1;j<=1;j++)
        { 
            vec2 texelOffset=vec2(halfMultiplier*i*texelSizeS,halfMultiplier*j*texelSizeT);
            texColour += texture(texture0, texCoord+texelOffset);
            count++;
        }


	fragColour = texColour/count; //calculate average divide by count which will = 9
}

