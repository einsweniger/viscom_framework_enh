#version 430 core

#include "../bicubic_sampling.glsl"

uniform sampler2D sourceTex;
uniform sampler2D blurTex[3];
uniform float bloomIntensity;

in vec2 texCoord;
out vec4 outColor;

vec4 pyramidFilter(sampler2D srcTex, vec2 tex, vec2 defocusWidth) {
    vec4 color = texture(srcTex, tex + vec2(0.5, 0.5) * defocusWidth);
    color += texture(srcTex, tex + vec2(-0.5,  0.5) * defocusWidth);
    color += texture(srcTex, tex + vec2( 0.5, -0.5) * defocusWidth);
    color += texture(srcTex, tex + vec2(-0.5, -0.5) * defocusWidth);
    return 0.25 * color;
}

const float w[3] = float[3](1.0, 3.0, 8.0);

void main() {
    ivec2 iTexCoord = ivec2(texCoord * vec2(textureSize(sourceTex, 0)));


    vec4 originalColor = texelFetch(sourceTex, iTexCoord, 0);
    vec3 glare = max(originalColor.rgb - vec3(1.0), 0.0);

    outColor = originalColor;
    outColor.rgb -= glare;
    for (int i = 0; i < 3; i++) {
        vec4 passSmple = sampleBiCubic(blurTex[i], texCoord);
        outColor.rgb += bloomIntensity * w[i] * passSmple.rgb / 12.0;
    }
}
