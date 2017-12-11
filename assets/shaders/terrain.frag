#version 440 core

layout (binding=0) uniform sampler2DArray tex;

in XFormed {
  smooth vec3 pos;
  smooth vec3 tcs;
  smooth vec3 norm;
} xd;

out vec4 frag;

void main () {
  vec4 texel = texture (tex, xd.tcs);
  float k = 0.05 + 0.95 * max (0, dot (xd.norm, normalize (vec3 (3, 2, 1))));
  frag = vec4 (k*texel.rgb, 0.5);
}

