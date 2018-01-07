#version 330 core

uniform sampler2D cocTex;
uniform sampler2D cocNearBlurTex;
uniform sampler2D colorTex;
uniform sampler2D colorMulCoCFarTex;

uniform vec2 offsets[48];

in vec2 texCoord;

layout(location = 0) out vec4 nearField; // 3 channels
layout(location = 1) out vec4 farField; // 3 channels

// const vec2 offsets[48] = vec2[] {
//     2.0f * vec2(1.000000f, 0.000000f),
//     2.0f * vec2(0.707107f, 0.707107f),
//     2.0f * vec2(-0.000000f, 1.000000f),
//     2.0f * vec2(-0.707107f, 0.707107f),
//     2.0f * vec2(-1.000000f, -0.000000f),
//     2.0f * vec2(-0.707106f, -0.707107f),
//     2.0f * vec2(0.000000f, -1.000000f),
//     2.0f * vec2(0.707107f, -0.707107f),
// 
//     4.0f * vec2(1.000000f, 0.000000f),
//     4.0f * vec2(0.923880f, 0.382683f),
//     4.0f * vec2(0.707107f, 0.707107f),
//     4.0f * vec2(0.382683f, 0.923880f),
//     4.0f * vec2(-0.000000f, 1.000000f),
//     4.0f * vec2(-0.382684f, 0.923879f),
//     4.0f * vec2(-0.707107f, 0.707107f),
//     4.0f * vec2(-0.923880f, 0.382683f),
//     4.0f * vec2(-1.000000f, -0.000000f),
//     4.0f * vec2(-0.923879f, -0.382684f),
//     4.0f * vec2(-0.707106f, -0.707107f),
//     4.0f * vec2(-0.382683f, -0.923880f),
//     4.0f * vec2(0.000000f, -1.000000f),
//     4.0f * vec2(0.382684f, -0.923879f),
//     4.0f * vec2(0.707107f, -0.707107f),
//     4.0f * vec2(0.923880f, -0.382683f),
// 
//     6.0f * vec2(1.000000f, 0.000000f),
//     6.0f * vec2(0.965926f, 0.258819f),
//     6.0f * vec2(0.866025f, 0.500000f),
//     6.0f * vec2(0.707107f, 0.707107f),
//     6.0f * vec2(0.500000f, 0.866026f),
//     6.0f * vec2(0.258819f, 0.965926f),
//     6.0f * vec2(-0.000000f, 1.000000f),
//     6.0f * vec2(-0.258819f, 0.965926f),
//     6.0f * vec2(-0.500000f, 0.866025f),
//     6.0f * vec2(-0.707107f, 0.707107f),
//     6.0f * vec2(-0.866026f, 0.500000f),
//     6.0f * vec2(-0.965926f, 0.258819f),
//     6.0f * vec2(-1.000000f, -0.000000f),
//     6.0f * vec2(-0.965926f, -0.258820f),
//     6.0f * vec2(-0.866025f, -0.500000f),
//     6.0f * vec2(-0.707106f, -0.707107f),
//     6.0f * vec2(-0.499999f, -0.866026f),
//     6.0f * vec2(-0.258819f, -0.965926f),
//     6.0f * vec2(0.000000f, -1.000000f),
//     6.0f * vec2(0.258819f, -0.965926f),
//     6.0f * vec2(0.500000f, -0.866025f),
//     6.0f * vec2(0.707107f, -0.707107f),
//     6.0f * vec2(0.866026f, -0.499999f),
//     6.0f * vec2(0.965926f, -0.258818f),
// };

vec4 calcNear(vec4 color, float cocNearBlurred, vec2 pixelSize)
{
    vec4 result = color;

    for (int i = 0; i < 48; i++) {
        vec2 offset = cocNearBlurred * offsets[i] * pixelSize;
        result += texture(colorTex, texCoord + offset); // maybe premultiply with near coc?
    }

    return result / 49.0f;
}

vec4 calcFar(ivec2 iTexCoord, float cocFar, vec2 pixelSize)
{
    vec4 result = texelFetch(colorMulCoCFarTex, iTexCoord, 0);
    float weightsSum = cocFar;

    for (int i = 0; i < 48; i++) {
        vec2 offset = cocFar * offsets[i] * pixelSize;

        float coc = texture(cocTex, texCoord + offset).y;
        vec4 color = texture(colorMulCoCFarTex, texCoord + offset);

        result += color;
        weightsSum += coc;
    }

    return result / weightsSum;
}

void main()
{
    vec2 texSize = vec2(textureSize(cocTex, 0));
    vec2 pixelSize = 1.0f / texSize;
    ivec2 iTexCoord = ivec2(texCoord * texSize);
    float cocNearBlurred = texelFetch(cocNearBlurTex, iTexCoord, 0).x;
    float cocFar = texelFetch(cocTex, iTexCoord, 0).y;
    vec4 color = texelFetch(colorTex, iTexCoord, 0);

    if (cocNearBlurred > 0.0f) nearField = calcNear(color, cocNearBlurred, pixelSize);
    else nearField = color;

    if (cocFar > 0.0f) farField = calcFar(iTexCoord, cocFar, pixelSize);
    else farField = vec4(0.0f);
}
