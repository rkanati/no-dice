#version 440 core

layout(binding=0) uniform sampler2D tex;

smooth in vec2 xd_tcs;

out vec4 frag;

void main () {
  vec4 texel = texture (tex, xd_tcs);
  frag = vec4 (1,1,1,texel.r);
}

