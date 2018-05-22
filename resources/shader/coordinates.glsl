
vec3 sphericalToCartesian(vec2 sph) {
    float phi = sph.x;
    float theta = sph.y;
    float sint = sin(theta);
    return normalize(vec3(sint * cos(phi), cos(theta), sint * sin(phi)));
}

vec2 cartesianToSpherical(vec3 cart) {
    float phi = atan(cart.z, cart.x);
    float theta = acos(cart.y);
    return vec2(phi, theta);
}
