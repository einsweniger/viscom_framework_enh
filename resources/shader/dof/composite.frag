#version 330 core

#include "../bicubic_sampling.glsl"

uniform sampler2D colorTex;
uniform sampler2D cocTex;
uniform sampler2D cocHalfTex;
uniform sampler2D cocNearBlurHalfTex;
uniform sampler2D dofNearHalfTex;
uniform sampler2D dofFarHalfTex;

uniform float blend;

in vec2 texCoord;

layout(location = 0) out vec4 result; // 3 channels

void main()
{
    vec2 texSize = vec2(textureSize(cocTex, 0));
    vec2 texSizeHalf = vec2(textureSize(cocHalfTex, 0));
    ivec2 iTexCoord = ivec2(texCoord * texSize);
    ivec2 iTexCoordHalf = ivec2(texCoord * texSizeHalf);

    result = texelFetch(colorTex, iTexCoord, 0);

    {
        // gather fetch:
        // 0   1 -> 2   3
        //   x   ->   x  
        // 3   2 -> 0   1
        float cocFar = texelFetch(cocTex, iTexCoord, 0).y;
        vec4 cocFarHalf = textureGather(cocHalfTex, texCoord, 1).wzxy;
        vec4 cocFarDiffs = abs(cocFar.xxxx - cocFarHalf);

        dofFar = sampleBiCubicBilateral(dofFarHalfTex, texCoord, cocFarDiffs);

        float blendWeight = blend * cocFar; // should use some non-linear factor here...
        result = mix(result, dofFar, blendWeight);
    }

    {
        float cocNear = sampleBiCubic(cocNearBlurHalfTex, texCoord).x;
        vec4 dofNear = sampleBiCubic(dofNearHalfTex, texCoord).x;

        float blendWeight = blend * cocNear; // should use some non-linear factor here...
        result = mix(result, dofNear, blendWeight);
    }
}
