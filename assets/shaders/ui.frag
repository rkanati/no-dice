#version 440 core

layout(location=101) uniform vec4 modulate;

layout(binding=0) uniform sampler2D tex;

smooth in vec2 xd_tcs;

out vec4 frag;

void main () {
  vec4 texel = texture (tex, xd_tcs);
  frag = modulate * texel;
}

