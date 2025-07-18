#version 460 core

precision mediump float;
precision mediump int;
precision mediump sampler2DArray;

layout (constant_id = 0) const int c_maxTextures = 4096;

layout(location = 0) in vec4 Color;
layout(location = 1) in vec3 UV;
layout(location = 2) in flat uint TextureIndex;

layout(set = 0, binding = 0) uniform sampler2DArray textureSampler[c_maxTextures];

layout(location = 0) out vec4 fragColor;

void main()
{	
    vec4 tex = texture(textureSampler[TextureIndex], UV);
    fragColor = Color*tex;
}