#version 330 core

uniform sampler2D sourceTex;

in vec2 texCoord;

layout(location = 0) out vec4 dsResult; // 4 channels

void main() {
    ivec2 iTexCoord = ivec2(texCoord * vec2(textureSize(sourceTex, 0)));

    ivec2 iTexCoordA[4];
    iTexCoordA[0] = iTexCoord - ivec2(-1, -1);
    iTexCoordA[1] = iTexCoord - ivec2( 0, -1);
    iTexCoordA[2] = iTexCoord - ivec2(-1,  0);
    iTexCoordA[3] = iTexCoord - ivec2( 0,  0);

    dsResult = vec4(0.0);
    for (int i = 0; i <= 3; ++i) {
        dsResult += texelFetch(sourceTex, iTexCoordA[i], 0).rgba;
    }

    if (dsResult.a > 1.0) dsResult /= dsResult.a;
}
