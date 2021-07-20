#version 400
in float data;
in float shading_amount;
out vec4 frag_color;
void main() {
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