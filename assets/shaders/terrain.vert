#version 440 core

layout (location=1) uniform mat4 w2c;

layout (location=1) in vec3 attr_pos;
layout (location=2) in int  attr_dir;
layout (location=3) in int  attr_tex;

out XFormed {
  smooth vec3 pos;
  smooth vec3 tcs;
  smooth vec3 norm;
} xd;

void main () {
/*const vec3 x_tab[6] = vec3[6] (
    vec3(1,0,0), vec3(-1,0,0), vec3(0,1,0),
    vec3(1,0,0), vec3(1,0,0), vec3(0,-1,0)
  );
  const vec3 y_tab[6] = vec3[6] (
    vec3(0,1,0), vec3(0,0,1), vec3(0,0,1),
    vec3(0,-1,0), vec3(0,0,1), vec3(0,0,1)
  );
  vec3 dx = x_tab[attr_dir], dy = y_tab[attr_dir];
  vec3 disp_tab[4] = vec3[4] ( vec3(0,0,0), dy, dy-dx, -dx );
  vec3 pos = attr_pos + disp_tab[gl_VertexID & 3];*/

  gl_Position = w2c * vec4 (attr_pos, 1);
  xd.pos = gl_Position.xyz;

  vec3 p = attr_pos;
  vec2 tc_tab[6] = vec2[6] (
    p.xy,
    vec2(-p.x,p.z),
    p.yz,
    vec2(p.x,-p.y),
    p.xz,
    vec2(-p.y,p.z)
  );
  vec2 tcs = (1.f/16.f)*tc_tab[attr_dir];
  xd.tcs = vec3 (tcs, attr_tex);

  const vec3 norm_tab[6] = vec3[6] (
    vec3(0,0, 1), vec3(0, 1,0), vec3( 1,0,0),
    vec3(0,0,-1), vec3(0,-1,0), vec3(-1,0,0)
  );
  xd.norm = norm_tab[attr_dir];
}

