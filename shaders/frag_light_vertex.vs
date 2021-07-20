#version 400
layout (location = 0) in vec3 vertex_position;
layout (location = 1) in float vertex_data;
layout (location = 2) in vec3 vertex_normal;

uniform mat4 MVP;

out float data;
out vec3 position;
out vec3 normal;

void main() {
  data = vertex_data;
  position = vertex_position;
  normal = vertex_normal;

  gl_Position = MVP * vec4(vertex_position, 1.0);
}