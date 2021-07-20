#version 400

in float data;
in vec3 position;
in vec3 normal;

uniform vec3 cameraloc;  // Camera position
uniform vec3 lightdir;   // Lighting direction
uniform vec4 lightcoeff; // Lighting coeff, Ka, Kd, Ks, alpha

out vec4 frag_color;

void main() {
  float diffuse = abs(dot(lightdir, normal));
  vec3 V = cameraloc - position;
  V = normalize(V);
  vec3 R = 2.0 * diffuse * normal - lightdir;
  R = normalize(R);
  float specular = pow(max(0.0, dot(V, R)), lightcoeff.w);
  diffuse = max(diffuse, 0.0);
  float shading_amount =
  lightcoeff.x +                // ambient
  (lightcoeff.y * diffuse) +    // diffuse
  (lightcoeff.z * specular);    // specular

  frag_color = vec4(1.0, 0.0, 0.0, 1.0);
  vec4 color1 = vec4(0.25, 0.25, 1.0, 1.0);
  vec4 color2 = vec4(1.0, 1.0, 1.0, 1.0);
  vec4 color3 = vec4(1.0, 0.25, 0.25, 1.0);
  float cutoff = 0.15;
  if(data >=0.0 && data <= cutoff) {
    float prop = (data - 0.0) / (cutoff - .0);
    frag_color = mix(color1, color2, prop);
  }
  else if(data > cutoff && data <= 1.0) {
    float prop = (data - cutoff) / (1.0-cutoff);
    frag_color = mix(color2, color3, prop);
  }
  frag_color = frag_color * shading_amount;
}