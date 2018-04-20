// shadertype=glsl
#version 330 core

uniform sampler2D sourceTex;
uniform float bloomWidth;

in vec2 texCoord;
out vec4 outColor;

#include "../gaussian_blur.glsl"

void main() {
#ifdef HORIZONTAL
    vec2 offset = vec2(bloomWidth/textureSize(sourceTex, 0).x, 0);
#endif
#ifdef VERTICAL
    vec2 offset = vec2(0, bloomWidth/textureSize(sourceTex, 0).y);
#endif

    vec3 blurColor = gaussianBlur(texCoord, offset, sourceTex);
    outColor = vec4(blurColor, 1.0);
}
