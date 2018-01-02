#version 330 core

uniform sampler2D colorTex;
uniform sampler2D cocTex;

uniform vec2 depthG;

uniform vec2 projParams;
uniform float focusZ;
uniform float apertureRadius;
uniform float fStops;
uniform vec2 cocMax;

in vec2 texCoord;

layout(location = 0) out vec4 colorHalf;
layout(location = 1) out vec4 colorMulCoCFarHalf;
layout(location = 2) out vec4 cocHalf;

void main()
{
    vec2 pixelSize = 1.0f / vec2(textureSize(cocTex, 0));

    vec2 texCoordA[4];
    texCoordA[0] = texCoord + vec2(-0.25f, -0.25f)*pixelSize;
    texCoordA[1] = texCoord + vec2( 0.25f, -0.25f)*pixelSize;
    texCoordA[2] = texCoord + vec2(-0.25f,  0.25f)*pixelSize;
    texCoordA[3] = texCoord + vec2( 0.25f,  0.25f)*pixelSize;

    vec3 colorA[4];
    float depthA[4];
    float cocFarA[4];
    float depthMin = 1000000000.0f;
    cocHalf = vec4(0.0f, 1000000000.0f, 0.0f, 0.0f);
    for (int i = 0; i < 3; ++i) {
        colorA[i] = texture(colorTex, texCoordA[i]).rgb;
        vec3 coc = texture(cocTex, texCoordA[i]).rgb;
        depthA[i] = coc.z;
        cocFarA[i] = coc.y;
        depthMin = min(depthMin, depthA[i]);
        cocHalf.x = max(cocHalf.x, coc.x); // near
        cocHalf.y = min(cocHalf.y, cocFarA[i]); // far
    }

    float depthWSum = 0.0f;
    float cocWSum = 0.0f;
    colorHalf = vec4(0.0f);
    colorMulCoCFarHalf = vec4(0.0f);
    for (int i = 0; i < 3; ++i) {
        float depthW = 1.0f / (abs(depthA[i] - depthMin) + 0.001f);
        depthWSum += depthW;
        colorHalf.xyz += depthW * colorA[i];

        float cocW = 1.0f / (abs(cocFarA[i] - cocHalf.y) + 0.001f);
        cocWSum += cocW;
        colorMulCoCFarHalf.xyz += cocW * colorA[i];
    }
    colorHalf.xyz /= depthWSum;
    colorMulCoCFarHalf.xyz /= cocWSum;
    colorMulCoCFarHalf.xyz *= cocHalf.y;
}
