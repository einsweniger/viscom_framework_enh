#version 330 core

uniform sampler2D colorTex;
uniform sampler2D cocTex;

in vec2 texCoord;

layout(location = 0) out vec4 colorHalf; // 3 channels
layout(location = 1) out vec4 colorMulCoCFarHalf; // 3 channels
layout(location = 2) out vec4 cocHalf; // 2 channels

void main()
{
    ivec2 iTexCoord = ivec2(texCoord * vec2(textureSize(colorTex, 0)));

    ivec2 iTexCoordA[4];
    iTexCoordA[0] = iTexCoord - ivec2(-1, -1);
    iTexCoordA[1] = iTexCoord - ivec2( 0, -1);
    iTexCoordA[2] = iTexCoord - ivec2(-1,  0);
    iTexCoordA[3] = iTexCoord - ivec2( 0,  0);

    vec3 colorA[4];
    float depthA[4];
    float cocFarA[4];
    float depthMin = 1000000000.0f;
    cocHalf = vec4(0.0f, 1000000000.0f, 0.0f, 0.0f);
    for (int i = 0; i <= 3; ++i) {
        colorA[i] = texelFetch(colorTex, iTexCoordA[i], 0).rgb;
        vec3 coc = texelFetch(cocTex, iTexCoordA[i], 0).rgb;
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
    for (int i = 0; i <= 3; ++i) {
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
