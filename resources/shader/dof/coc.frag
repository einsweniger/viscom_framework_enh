#version 330 core

uniform sampler2D depthTex;
uniform vec2 projParams;
uniform float focusZ;
uniform float apertureRadius;
uniform float fStops;
uniform vec2 cocMax;

in vec2 texCoord;

layout(location = 0) out vec4 cocResult; // 3 channels

float depthNDCToView(float depthNCD)
{
    return -projParams.y / (depthNCD + projParams.x);
}

void main()
{
    ivec2 iTexCoord = ivec2(texCoord * vec2(textureSize(depthTex, 0)));
    float depthNDC = texelFetch(depthTex, iTexCoord, 0).x;
    float depth = -depthNDCToView(depthNDC);

    float focalLength = fStops * 2.0f * apertureRadius;
    float coc = focusZ - depth;
    coc *= focalLength * 2.0f * apertureRadius;
    coc /= depth * (focusZ - focalLength);

    cocResult = vec4(0.0f);
    if (coc < 0.0f) cocResult.y = -coc; // far
    else cocResult.x = coc; // near
    cocResult.xy /= cocMax;
    cocResult = clamp(cocResult, 0.0f, 1.0f);
    cocResult.z = depth;
}
