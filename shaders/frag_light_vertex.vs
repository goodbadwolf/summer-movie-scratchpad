#version 400
layout (location = 0) in vec3 vertex_position;
layout (location = 1) in float vertex_data;
layout (location = 2) in vec3 vertex_normal;
uniform mat4 MVP;
uniform vec3 cameraloc;  // Camera position
uniform vec3 lightdir;   // Lighting direction
uniform vec4 lightcoeff; // Lighting coeff, Ka, Kd, Ks, alpha
out float data;
out float shading_amount;
// out vec3 v_position;
// out vec3 v_normal;
void main() {
  data = vertex_data;
  float diffuse = dot(lightdir, vertex_normal);
  vec3 V = cameraloc - vertex_position;
  V = normalize(V);
  vec3 R = 2.0 * diffuse * vertex_normal - lightdir;
  R = normalize(R);
  float specular = pow(max(0.0, dot(V, R)), lightcoeff.w);
  diffuse = max(diffuse, 0.0);
  shading_amount =
  lightcoeff.x +                // ambient
  (lightcoeff.y * diffuse) +    // diffuse
  (lightcoeff.z * specular);    // specular

  vec4 position = vec4(vertex_position, 1.0);
  gl_Position = MVP*position;
}