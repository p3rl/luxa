// VERTEX SHADER
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;

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
    //gl_Position = vec4(inPosition, 0.0, 1.0) * ubo.model * ubo.view * ubo.proj;
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
    //gl_Position = vec4(inPosition, 0.0, 1.0);
    fragColor =  vec3(1.0, 1.0, 0.0);
}

// out gl_PerVertex {
//     vec4 gl_Position;
// };

// layout(location = 0) out vec3 fragColor;

// vec2 positions[3] = vec2[](
//     vec2(0.0, -0.5),
//     vec2(0.5, 0.5),
//     vec2(-0.5, 0.5)
// );

// vec3 colors[3] = vec3[](
//     vec3(1.0, 0.0, 0.0),
//     vec3(0.0, 1.0, 0.0),
//     vec3(0.0, 0.0, 1.0)
// );

// void main() {
//     gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
//     fragColor = colors[gl_VertexIndex];
// }