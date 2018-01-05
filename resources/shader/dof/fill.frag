#version 330 core

uniform sampler2D cocTex;
uniform sampler2D cocNearBlurTex;
uniform sampler2D dofNearTex;
uniform sampler2D dofFarTex;

in vec2 texCoord;

layout(location = 0) out vec4 nearField; // 3 channels
layout(location = 1) out vec4 farField; // 3 channels

void main()
{
    vec2 texSize = vec2(textureSize(cocTex, 0));
    ivec2 iTexCoord = ivec2(texCoord * texSize);
    float cocNearBlurred = texelFetch(cocNearBlurTex, iTexCoord, 0).x;
    float cocFar = texelFetch(cocTex, iTexCoord, 0).y;

    nearField = texelFetch(dofNearTex, iTexCoord, 0);
    farField = texelFetch(dofFarTex, iTexCoord, 0);

    if (cocNearBlurred > 0.0f) {
        for (int i = -1; i <= 1; ++i) {
            for (int j = -1; j <= 1; ++j) {
                vec4 nearColor = texelFetch(dofNearTex, iTexCoord + ivec2(i, j), 0);
                nearField = max(nearField, nearColor);
            }
        }
    }

    if (cocFar > 0.0f) {
        for (int i = -1; i <= 1; ++i) {
            for (int j = -1; j <= 1; ++j) {
                vec4 farColor = texelFetch(dofFarTex, iTexCoord + ivec2(i, j), 0);
                farField = max(farField, farColor);
            }
        }
    }
}
