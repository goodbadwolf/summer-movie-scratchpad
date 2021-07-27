#version 400

in float data;
in vec3 position;
in vec3 normal;

uniform vec3 cameraloc;  // Camera position
uniform vec3 lightdir;   // Lighting direction
uniform vec3 lightloc;   // Lighting direction
uniform vec4 lightcoeff; // Lighting coeff, Ka, Kd, Ks, alpha
uniform float WIDTH;
uniform float HEIGHT;
uniform sampler2D modelTexture;

out vec4 frag_color;

float ggx (vec3 N, vec3 V, vec3 L, float roughness, float F0) {
  float alpha = roughness*roughness;
  vec3 H = normalize(L - V);
  float dotLH = max(0.0, dot(L,H));
  float dotNH = max(0.0, dot(N,H));
  float dotNL = max(0.0, dot(N,L));
  float alphaSqr = alpha * alpha;
  float denom = dotNH * dotNH * (alphaSqr - 1.0) + 1.0;
  float D = alphaSqr / (3.141592653589793 * denom * denom);
  float F = F0 + (1.0 - F0) * pow(1.0 - dotLH, 5.0);
  float k = 0.5 * alpha;
  float k2 = k * k;
  return dotNL * D * F / (dotLH*dotLH*(1.0-k2)+k2);
}

void main() {
  vec3 N = normal;
  vec3 V = normalize(cameraloc - position);
  vec3 L = normalize(lightloc - position);
  float spec = max(0.0, ggx(N, V, L, 0.1, 0.75));

  /*
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
  */

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

  //frag_color = vec4(1,1,1, 1.0);
  float u = (gl_FragCoord.x - 0.5f) / (WIDTH - 1.0) ;
  float v = (gl_FragCoord.y - 0.5f) / (HEIGHT - 1.0);
  vec4 color = texture(modelTexture, vec2(u, v));
  color = vec4(0.8, 0.1, 0.1, 1.0);
  float shading_amount = spec;
  frag_color = color * shading_amount;
  frag_color = pow(frag_color, vec4(2.2));
}