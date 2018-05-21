
vec3 sphericalToCartesian(vec2 sph) {
    float phi = sph.x;
    float theta = sph.y;
    float sint = sin(theta);
    return normalize(vec3(sint * cos(phi), cos(theta), sint * sin(phi)));
}
