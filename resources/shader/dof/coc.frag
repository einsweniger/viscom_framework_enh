#version 330 core

uniform sampler2D depthTex;
uniform vec2 projParams;
uniform vec2 cocParams;

in vec2 texCoord;

layout(location = 0) out vec4 cocResult; // 3 channels

float depthNDCToView(float depthNDC)
{
    return projParams.y / (depthNDC + projParams.x);
}

void main()
{
    ivec2 iTexCoord = ivec2(texCoord * vec2(textureSize(depthTex, 0)));
    float depthNDC = (2.0f * texelFetch(depthTex, iTexCoord, 0).x) - 1.0f;

    float coc = (depthNDC * cocParams.x) + cocParams.y;

    cocResult = vec4(0.0f);
    if (coc < 0.0f) cocResult.y = clamp(-coc, 0.0, 1.0); // far
    else cocResult.x = clamp(coc, 0.0, 1.0); // near
    cocResult.z = depthNDCToView(depthNDC);
}
