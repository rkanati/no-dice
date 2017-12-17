#version 440 core

layout (location=1) uniform vec2 ui_scale;
layout (location=2) uniform vec2 ui_offset;

layout (binding=0) uniform sampler2D tex;

layout (location=1) in vec2 attr_pos;
layout (location=2) in vec2 attr_uv;

smooth out vec2 xd_tcs;

void main () {
  vec2 pos = ui_scale*(attr_pos + ui_offset) - vec2(1,1);
  gl_Position = vec4 (pos.x, pos.y, -1, 1);
  xd_tcs = attr_uv/textureSize (tex, 0);
}

