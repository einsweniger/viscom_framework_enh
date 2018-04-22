#version 430 core

struct FilmicParams
{
    float sStrength;
    float linStrength;
    float linAngle;
    float toeStrength;
    float toeNumerator;
    float toeDenominator;
    float white;
    float exposure;
};


layout(std140) uniform filmicBuffer
{
    FilmicParams filmicParams;
};

uniform sampler2D sourceTex;

in vec2 texCoord;

out vec4 outputColor;


vec3 rgb2Yuv(vec3 c)
{
    vec3 result;
    result.r = 0.299f * c.r + 0.587f * c.g + 0.114f * c.b;
    result.g = (c.b - result.r) * 0.565f;
    result.b = (c.r - result.r)*0.713f;
    return result;
}

vec3 yuv2Rgb(vec3 c)
{
    vec3 result;
    result.r = c.r + 1.403f * c.b;
    result.g = c.r - 0.344f * c.g - 0.714f * c.b;
    result.b = c.r + 1.770f * c.g;
    return result;
}

vec3 Uncharted2Tonemap(vec3 x)
{
    vec3 Ax = filmicParams.sStrength*x;
    float toeAngle = filmicParams.toeNumerator / filmicParams.toeDenominator;
    return ((x*(Ax + filmicParams.linAngle*filmicParams.linStrength) + filmicParams.toeStrength*filmicParams.toeNumerator)
        / (x*(Ax + filmicParams.linStrength) + filmicParams.toeStrength*filmicParams.toeDenominator)) - toeAngle;
}

void main() {
    vec4 rgbaVal = texture(sourceTex, texCoord);

    vec3 curr = 2.0f * Uncharted2Tonemap(filmicParams.exposure*rgbaVal.rgb);
    vec3 whiteScale = 1.0f / Uncharted2Tonemap(vec3(filmicParams.white));
    vec3 color = curr*whiteScale;

    outputColor = vec4(color, rgbaVal.a);
}
