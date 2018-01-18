#version 330
#extension GL_ARB_separate_shader_objects : require

layout(location = 0) in vec4 position;

uniform mat4 vpMatrix;
uniform mat4 modelMatrix;

uniform float pointSize = 1.0f;

layout(location = 0) out vec3 vPos;

void main()
{
    vec4 posV4 = modelMatrix * vec4(position.xyz, 1);
    vPos = vec3(posV4);

    gl_Position = vpMatrix * posV4;
    gl_PointSize = pointSize;
}
