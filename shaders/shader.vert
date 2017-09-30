// VERTEX SHADER
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;


out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    mat4 mvp = ubo.proj * ubo.view * ubo.model;
    //mat4 mvp = transpose(ubo.model * ubo.view * ubo.proj);
    //gl_Position =  vec4(inPosition, 1.0) * transpose(mvp);
    gl_Position = mvp * vec4(inPosition, 1.0);
    gl_Position.y = -gl_Position.y;
    gl_Position.z = (gl_Position.z + gl_Position.w) / 2.0;
    fragColor =  inColor;
}