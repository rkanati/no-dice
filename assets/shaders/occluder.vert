#version 440 core

layout (location=1) uniform mat4 w2c;

layout (location=1) in vec3 attr_pos;

void main () {
  gl_Position = w2c * vec4 (attr_pos, 1);
}

