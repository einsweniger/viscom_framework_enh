#version 330 core

uniform sampler2D colorTex;
uniform sampler2D cocTex;
uniform sampler2D cocHalfTex;
uniform sampler2D cocNearBlurHalfTex;
uniform sampler2D dofNearHalfTex;
uniform sampler2D dofFarHalfTex;

uniform sampler1D hgTex;

in vec2 texCoord;

layout(location = 0) out vec4 result; // 3 channels

vec4 sampleBiCubic(sampler2D tex, vec2 coords) {
    vec2 texSize = vec2(textureSize(tex, 0));
    vec2 pixelSize = 1.0f / texSize;
    vec2 pixelSizeX = vec2(pixelSize.x, 0.0f);
    vec2 pixelSizeY = vec2(0.0f, pixelSize.y);
    vec2 coordHG = coords * texSize - vec2(0.5f, 0.5f);
    vec3 hgX = texture(hgTex, coordHG.x);
    vec3 hgY = texture(hgTex, coordHG.y);

    vec2 coordsA[4];
    coordsA[0] = coords + hgX.x * pixelSizeX;
    coordsA[1] = coords - hgX.y * pixelSizeX;
    coordsA[2] = coordsA[0] + hgY.x * pixelSizeY;
    coordsA[3] = coordsA[1] + hgY.x * pixelSizeY;
    coordsA[0] = coordsA[0] - hgY.y * pixelSizeY;
    coordsA[1] = coordsA[1] - hgY.y * pixelSizeY;

    vec4 texSourceA[4];
    texSourceA[0] = texture(tex, coordsA[1]);
    texSourceA[1] = texture(tex, coordsA[0]);
    texSourceA[2] = texture(tex, coordsA[3]);
    texSourceA[3] = texture(tex, coordsA[2]);

    texSourceA[0] = mix(texSourceA[0], texSourceA[2], hgY.z);
    texSourceA[1] = mix(texSourceA[1], texSourceA[3], hgY.z);

    return mix(texSourceA[0], texSourceA[1], hgX.z);
}

vec4 sampleBiCubicBilateral(sampler2D tex, vec2 coords, vec4 bilateralWeights) {
    vec2 texSize = vec2(textureSize(tex, 0));
    vec2 pixelSize = 1.0f / texSize;
    vec2 pixelSizeX = vec2(pixelSize.x, 0.0f);
    vec2 pixelSizeY = vec2(0.0f, pixelSize.y);
    vec2 coordHG = coords * texSize - vec2(0.5f, 0.5f);
    vec3 hgX = texture(hgTex, coordHG.x);
    vec3 hgY = texture(hgTex, coordHG.y);

    // 3   2
    //   x  
    // 1   0
    vec2 coordsA[4];
    coordsA[0] = coords + hgX.x * pixelSizeX;
    coordsA[1] = coords - hgX.y * pixelSizeX;
    coordsA[2] = coordsA[0] + hgY.x * pixelSizeY;
    coordsA[3] = coordsA[1] + hgY.x * pixelSizeY;
    coordsA[0] = coordsA[0] - hgY.y * pixelSizeY;
    coordsA[1] = coordsA[1] - hgY.y * pixelSizeY;

    vec4 texSourceA[4];
    texSourceA[0] = texture(tex, coordsA[1]);
    texSourceA[1] = texture(tex, coordsA[0]);
    texSourceA[2] = texture(tex, coordsA[3]);
    texSourceA[3] = texture(tex, coordsA[2]);

    float weightsA[4]; // bilateral?
    weightsA[0] = ((1.0f - hgY.z) * (1.0f - hgX.z)) / (bilateralWeights.x + 0.001f);
    weightsA[1] = ((1.0f - hgY.z) * hgX.z) / (bilateralWeights.y + 0.001f);
    weightsA[2] = (hgY.z * (1.0f - hgX.z)) / (bilateralWeights.z + 0.001f);
    weightsA[0] = (hgY.z * hgX.z) / (bilateralWeights.w + 0.001f);

    float weightsSum = weightsA[0] + weightsA[1] + weightsA[2] + weightsA[3];

    vec4 result = texSourceA[0] * weightsA[0];
    result += texSourceA[1] * weightsA[1];
    result += texSourceA[2] * weightsA[2];
    result += texSourceA[3] * weightsA[3];

    return result / weightsSum;
}

void main()
{
    vec2 texSize = vec2(textureSize(cocTex, 0));
    vec2 texSizeHalf = vec2(textureSize(cocHalfTex, 0));
    ivec2 iTexCoord = ivec2(texCoord * texSize);
    ivec2 iTexCoordHalf = ivec2(texCoord * texSizeHalf);

    result = texelFetch(colorTex, iTexCoord, 0);

    {

    }

    {
        float cocNear = sampleBiCubic(cocNearBlurHalfTex, texCoord).x;
        vec4 dofNear = sampleBiCubic(dofNearHalfTex, texCoord).x;

        float blendWeight = blend * cocNear; // should use some non-linear factor here...
        result = mix(result, dofNear, blendWeight);
    }
}
