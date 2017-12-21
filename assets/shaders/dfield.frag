#version 440 core

layout(location=1) uniform float threshold;
layout(location=2) uniform float border;
layout(location=3) uniform vec4 border_colour;
layout(location=4) uniform vec4 inner_colour;

layout(binding=0) uniform sampler2D dfield;

smooth sample in vec2 xd_tcs;

out vec4 frag;

void main () {
  vec4 texel = texture (dfield, xd_tcs);
  if (texel.r < threshold)
    discard;
  else if (texel.r < threshold + border)
    frag = border_colour;
  else
    frag = inner_colour;
}

