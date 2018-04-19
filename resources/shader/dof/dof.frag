#version 330 core

uniform sampler2D cocTex;
uniform sampler2D cocNearBlurTex;
uniform sampler2D colorTex;
uniform sampler2D colorMulCoCFarTex;

uniform vec2 offsets[48];

in vec2 texCoord;

layout(location = 0) out vec4 nearField; // 3 channels
layout(location = 1) out vec4 farField; // 3 channels

vec4 calcNear(vec4 color, float cocNearBlurred, vec2 pixelSize)
{
    vec4 result = color;

    for (int i = 0; i < 48; i++) {
        vec2 offset = cocNearBlurred * offsets[i] * pixelSize;
        result += texture(colorTex, texCoord + offset);
    }

    return result / 49.0f;
}

vec4 calcFar(ivec2 iTexCoord, float cocFar, vec2 pixelSize)
{
    vec4 result = texelFetch(colorMulCoCFarTex, iTexCoord, 0);
    float weightsSum = 0.0;

    for (int i = 0; i < 48; i++) {
        vec2 offset = clamp(cocFar, 0.0, 1.0) * offsets[i] * pixelSize;

        float coc = clamp(texture(cocTex, texCoord + offset).y, 0.0, 1.0);
        vec4 color = texture(colorMulCoCFarTex, texCoord + offset);

        result += coc * color;
        weightsSum += coc;
    }

    return result / weightsSum;
}

void main()
{
    vec2 texSize = vec2(textureSize(cocTex, 0));
    vec2 pixelSize = 1.0f / texSize;
    ivec2 iTexCoord = ivec2(texCoord * texSize);
    float cocNearBlurred = clamp(texelFetch(cocNearBlurTex, iTexCoord, 0).x, 0.0, 1.0);
    float cocFar = clamp(texelFetch(cocTex, iTexCoord, 0).y, 0.0, 1.0);
    vec4 color = texelFetch(colorTex, iTexCoord, 0);
    
    if (cocNearBlurred > 0.0f) nearField = cocNearBlurred * calcNear(color, cocNearBlurred, pixelSize);
    else nearField = cocNearBlurred * color;

    if (cocFar > 0.0f) farField = cocFar * calcFar(iTexCoord, cocFar, pixelSize);
    else farField = vec4(0.0f);
}
