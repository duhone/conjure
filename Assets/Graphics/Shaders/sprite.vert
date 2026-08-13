#version 460 core

precision highp float;

layout(location = 0) in vec2 Offset;
layout(location = 1) in uvec2 TextureFrame;
layout(location = 2) in vec4 ColorIn;
layout(location = 3) in uvec2 FrameSize;
layout(location = 4) in vec4 Rotation;

layout(location = 0) out mediump vec4 Color;
layout(location = 1) out mediump vec3 UV;
layout(location = 2) out flat mediump uint TextureIndex;

const vec2 Vertices[4] = vec2[4](vec2(0.0, 0.0), vec2(1.0, 0.0), vec2(0.0, 1.0), vec2(1.0, 1.0));

layout(set = 0, binding = 1) uniform GlobalUniformBuffer {
    vec2 InvScreenSize;
} globalUBO;

void main() {
  vec2 position = Vertices[gl_VertexIndex];
  UV.xy = position;
  UV.z = TextureFrame.y;
  
  mat2 rot = mat2(Rotation.x, Rotation.y, Rotation.z, Rotation.w);
  position = rot * (position-0.5);
  position += 0.5;

  position *= vec2(FrameSize);
  position += Offset;

  // from pixel coords to -1to1
  vec2 pos = (position * vec2(2.0) * globalUBO.InvScreenSize) - vec2(1.0);
  gl_Position = vec4(pos.x, pos.y, 0.0, 1.0);

  Color = ColorIn;
  TextureIndex = TextureFrame.x;
}