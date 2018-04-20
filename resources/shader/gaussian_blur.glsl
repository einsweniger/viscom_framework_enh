// blur algorithm from:
// https://software.intel.com/en-us/blogs/2014/07/15/an-investigation-of-fast-real-time-gpu-based-image-blur-algorithms
const float weights[2] = float[] (0.44908, 0.05092);
const float offsets[2] = float[] (0.53805, 2.06278);

vec3 gaussianBlur(vec2 p, vec2 offset, sampler2D blurTexture)
{
    vec3 result = vec3(0.0f);

    for (int i = 0; i < 2; ++i) {
        vec2 texOffset = offsets[i] * offset;
        vec3 color = texture(blurTexture, p + texOffset).xyz +
            texture(blurTexture, p - texOffset).xyz;
        result += weights[i] * color;
    }

    return result;
}
