#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <string>

#include <GL/glew.h>    // include GLEW and new version of GL on Windows
#include <GLFW/glfw3.h> // GLFW helper library

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using std::cerr;
using std::endl;

//#define VERTEX_LIGHTING
#define FRAGMENT_LIGHTING

void _print_shader_info_log(GLuint shader_index)
{
  int max_length = 2048;
  int actual_length = 0;
  char shader_log[2048];
  glGetShaderInfoLog(shader_index, max_length, &actual_length, shader_log);
  printf("shader info log for GL index %u:\n%s\n", shader_index, shader_log);
}

void GetTimeStep(int ts, float **tri_points, float **tri_normals, int **tri_indices, float **tri_data, int &num_points, int &num_tris)
{
  char filename[1024];
  sprintf(filename, "aneurysm/aneurysm%03d.obj", ts);
  FILE *f = fopen(filename, "r");
  if (f == NULL)
  {
    cerr << "Cannot open file " << filename << endl;
    exit(EXIT_FAILURE);
  }
  fseek(f, 0, SEEK_END);
  int numbytes = ftell(f);
  fseek(f, 0, SEEK_SET);
  char *buff = new char[numbytes];
  fread(buff, sizeof(char), numbytes, f);
  fclose(f);
  std::vector<float> points;
  std::vector<float> normals;
  std::vector<int> indices;
  std::vector<float> data;
  char *curr = strtok(buff, "\n");
  while (curr != NULL)
  {
    if (curr[0] == '#')
    {
      // this is a comment: do nothing
    }
    else if (curr[0] == '\0')
    {
      // this is an empty line: do nothing
    }
    else if (curr[0] == 'v' && curr[1] == ' ')
    {
      // vertex
      float vx, vy, vz;
      sscanf(curr + 2, "%f %f %f", &vx, &vy, &vz);
      // scale and translate into the cube X=-10->+10, Y=-10->+10, Z=-10->+10
      points.push_back((vx - 2.5) / (2.8) * 20 - 10);
      points.push_back((vy - 2.7) / (5.5 - 2.7) * 20 - 10);
      points.push_back((vz - 3.5) / (2.8) * 20 - 10);
    }
    else if (curr[0] == 'v' && curr[1] == 'n' && curr[2] == ' ')
    {
      // normal
      float nx, ny, nz;
      sscanf(curr + 3, "%f %f %f", &nx, &ny, &nz);
      normals.push_back(nx);
      normals.push_back(ny);
      normals.push_back(nz);
    }
    else if (curr[0] == 'v' && curr[1] == 't' && curr[2] == ' ')
    {
      // data
      float nx1, nx2;
      sscanf(curr + 3, "%f %f", &nx1, &nx2);
      data.push_back(nx1);
    }
    else if (curr[0] == 'f' && curr[1] == ' ')
    {
      // indices
      int v1, v2, v3, v4, v5, v6, v7, v8, v9;
      sscanf(curr + 2, "%d/%d/%d %d/%d/%d %d/%d/%d", &v1, &v2, &v3, &v4, &v5, &v6, &v7, &v8, &v9);
      indices.push_back(v1 - 1);
      indices.push_back(v4 - 1);
      indices.push_back(v7 - 1);
    }
    curr = strtok(NULL, "\n");
  }
  *tri_points = new float[points.size()];
  memcpy(*tri_points, &(points[0]), sizeof(float) * points.size());
  *tri_normals = new float[normals.size()];
  memcpy(*tri_normals, &(normals[0]), sizeof(float) * normals.size());
  *tri_data = new float[data.size()];
  memcpy(*tri_data, &(data[0]), sizeof(float) * data.size());
  *tri_indices = new int[indices.size()];
  memcpy(*tri_indices, &(indices[0]), sizeof(int) * indices.size());
  num_points = points.size() / 3;
  num_tris = indices.size() / 3;
  delete[] buff;
}

struct ModelGLState
{
  bool initialized;
  GLuint vao;
  GLuint points_vbo;
  GLuint data_vbo;
  GLuint normals_vbo;
  GLuint index_ebo;

  ModelGLState()
      : initialized(false),
        vao(0),
        points_vbo(0),
        data_vbo(0),
        normals_vbo(0),
        index_ebo(0)
  {
  }
};

GLuint LoadAneurysmModel(int ts, int &num_tris, ModelGLState &glState)
{
  float *tri_points, *tri_normals, *tri_data;
  int *tri_indices;
  int num_points;
  GetTimeStep(ts, &tri_points, &tri_normals, &tri_indices, &tri_data, num_points, num_tris);

  if (!glState.initialized)
  {
    glGenVertexArrays(1, &glState.vao);
    GLuint bufferObjects[4];
    glGenBuffers(4, &bufferObjects[0]);
    glState.points_vbo = bufferObjects[0];
    glState.data_vbo = bufferObjects[1];
    glState.normals_vbo = bufferObjects[2];
    glState.index_ebo = bufferObjects[3];
  }

  glBindVertexArray(glState.vao);

  glBindBuffer(GL_ARRAY_BUFFER, glState.points_vbo);
  if (glState.initialized)
  {
    glBufferSubData(GL_ARRAY_BUFFER, 0, 3 * num_points * sizeof(float), tri_points);
  }
  else
  {
    glBufferData(GL_ARRAY_BUFFER, 3 * num_points * sizeof(float), tri_points, GL_DYNAMIC_DRAW);
  }

  glBindBuffer(GL_ARRAY_BUFFER, glState.data_vbo);
  if (glState.initialized)
  {
    glBufferSubData(GL_ARRAY_BUFFER, 0, num_points * sizeof(float), tri_data);
  }
  else
  {
    glBufferData(GL_ARRAY_BUFFER, num_points * sizeof(float), tri_data, GL_DYNAMIC_DRAW);
  }

  glBindBuffer(GL_ARRAY_BUFFER, glState.normals_vbo);
  if (glState.initialized)
  {
    glBufferSubData(GL_ARRAY_BUFFER, 0, 3 * num_points * sizeof(float), tri_normals);
  }
  else
  {
    glBufferData(GL_ARRAY_BUFFER, 3 * num_points * sizeof(float), tri_normals, GL_STATIC_DRAW);
  }

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glState.index_ebo);
  if (glState.initialized)
  {
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, 3 * num_tris * sizeof(GLuint), tri_indices);
  }
  else
  {
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * num_tris * sizeof(GLuint), tri_indices, GL_DYNAMIC_DRAW);
  }

  glBindBuffer(GL_ARRAY_BUFFER, glState.points_vbo);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
  glBindBuffer(GL_ARRAY_BUFFER, glState.data_vbo);
  glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, NULL);
  glBindBuffer(GL_ARRAY_BUFFER, glState.normals_vbo);
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, NULL);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glState.index_ebo);

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);

  delete[] tri_points;
  delete[] tri_data;
  delete[] tri_normals;
  delete[] tri_indices;

  glState.initialized = true;
  return glState.vao;
}

#ifdef VERTEX_LIGHTING
const char *vertexShader =
    "#version 400\n"
    "layout (location = 0) in vec3 vertex_position;\n"
    "layout (location = 1) in float vertex_data;\n"
    "layout (location = 2) in vec3 vertex_normal;\n"
    "uniform mat4 MVP;\n"
    "uniform vec3 cameraloc;  // Camera position \n"
    "uniform vec3 lightdir;   // Lighting direction \n"
    "uniform vec4 lightcoeff; // Lighting coeff, Ka, Kd, Ks, alpha\n"
    "out float data;\n"
    "out float shading_amount;\n"
    "void main() {\n"
    "  data = vertex_data;\n"
    "  float diffuse = dot(lightdir, vertex_normal);"
    "  vec3 V = cameraloc - vertex_position;"
    "  V = normalize(V);"
    "  vec3 R = 2.0 * diffuse * vertex_normal - lightdir;"
    "  R = normalize(R);"
    "  float specular = pow(max(0.0, dot(V, R)), lightcoeff.w);"
    "  diffuse = max(diffuse, 0.0);"
    "  shading_amount = "
    "  lightcoeff.x +                // ambient \n"
    "  (lightcoeff.y * diffuse) +    // diffuse \n"
    "  (lightcoeff.z * specular);    // specular\n"
    "  vec4 position = vec4(vertex_position, 1.0);\n"
    "  gl_Position = MVP*position;\n"
    "}\n";

const char *fragmentShader =
    "#version 400\n"
    "in float data;\n"
    "in float shading_amount;\n"
    "out vec4 frag_color;\n"
    "void main() {\n"
    "  frag_color = vec4(1.0, 0.0, 0.0, 1.0);\n"
    "  vec4 color1 = vec4(0.25, 0.25, 1.0, 1.0);"
    "  vec4 color2 = vec4(1.0, 1.0, 1.0, 1.0);"
    "  vec4 color3 = vec4(1.0, 0.25, 0.25, 1.0);"
    "  float cutoff = 0.15;"
    "  if(data >=0.0 && data <= cutoff) {"
    "    float prop = (data - 0.0) / (cutoff - .0);"
    "    frag_color = mix(color1, color2, prop);"
    "  }\n"
    "  else if(data > cutoff && data <= 1.0) {"
    "    float prop = (data - cutoff) / (1.0-cutoff);"
    "    frag_color = mix(color2, color3, prop);"
    "  }\n"
    "  frag_color = frag_color * shading_amount;"
    "}\n";
#endif

std::string ReadFile(std::string name)
{
  std::ifstream ifs(name);
  return std::string((std::istreambuf_iterator<char>(ifs)),
                     (std::istreambuf_iterator<char>()));
}

int main()
{
  // start GL context and O/S window using the GLFW helper library
  if (!glfwInit())
  {
    fprintf(stderr, "ERROR: could not start GLFW3\n");
    return 1;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window = glfwCreateWindow(1280, 1280, "Aneurysm", NULL, NULL);
  if (!window)
  {
    fprintf(stderr, "ERROR: could not open window with GLFW3\n");
    glfwTerminate();
    return 1;
  }
  glfwMakeContextCurrent(window);
  // start GLEW extension handler
  glewExperimental = GL_TRUE;
  glewInit();

  // get version info
  const GLubyte *renderer = glGetString(GL_RENDERER); // get renderer string
  const GLubyte *version = glGetString(GL_VERSION);   // version as a string
  printf("Renderer: %s\n", renderer);
  printf("OpenGL version supported %s\n", version);

  // tell GL to only draw onto a pixel if the shape is closer to the viewer
  glEnable(GL_DEPTH_TEST); // enable depth-testing
  glDepthFunc(GL_LESS);    // depth-testing interprets a smaller value as "closer"

  GLuint vao = 0;
  int num_tris;
#ifdef VERTEX_LIGHTING
  const char *vertex_shader = vertexShader;
  const char *fragment_shader = fragmentShader;
#elif defined(FRAGMENT_LIGHTING)
  std::string vertex_shader_str = ReadFile("shaders/frag_light_vertex.vs");
  std::string fragment_shader_str = ReadFile("shaders/frag_light_fragment.fs");
  const char *vertex_shader = vertex_shader_str.c_str();
  const char *fragment_shader = fragment_shader_str.c_str();
#endif

  GLuint vs = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vs, 1, &vertex_shader, NULL);
  glCompileShader(vs);
  int params = -1;
  glGetShaderiv(vs, GL_COMPILE_STATUS, &params);
  if (GL_TRUE != params)
  {
    fprintf(stderr, "ERROR: GL vertex shader index %i did not compile\n", vs);
    _print_shader_info_log(vs);
    exit(EXIT_FAILURE);
  }

  GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fs, 1, &fragment_shader, NULL);
  glCompileShader(fs);
  glGetShaderiv(fs, GL_COMPILE_STATUS, &params);
  if (GL_TRUE != params)
  {
    fprintf(stderr, "ERROR: GL fragment shader index %i did not compile\n", fs);
    _print_shader_info_log(fs);
    exit(EXIT_FAILURE);
  }

  GLuint shader_programme = glCreateProgram();
  glAttachShader(shader_programme, fs);
  glAttachShader(shader_programme, vs);
  glLinkProgram(shader_programme);

  glUseProgram(shader_programme);

  // Projection matrix : 30° Field of View
  // display size  : 1000x1000
  // display range : 5 unit <-> 200 units
  glm::mat4 Projection = glm::perspective(
      glm::radians(30.0f), (float)1000 / (float)1000, 5.0f, 200.0f);
  glm::vec3 camera(0, 40, 20);
  glm::vec3 origin(0, 0, 0);
  glm::vec3 up(0, 1, 0);
  // Camera matrix
  glm::mat4 View = glm::lookAt(
      camera, // Camera in world space
      origin, // looks at the origin
      up      // and the head is up
  );
  // Model matrix : an identity matrix (model will be at the origin)
  glm::mat4 Model = glm::mat4(1.0f);
  // Our ModelViewProjection : multiplication of our 3 matrices
  glm::mat4 mvp = Projection * View * Model;

  // Get a handle for our "MVP" uniform
  // Only during the initialisation
  GLuint mvploc = glGetUniformLocation(shader_programme, "MVP");
  // Send our transformation to the currently bound shader, in the "MVP" uniform
  // This is done in the main loop since each model will have a different MVP matrix
  // (At least for the M part)
  glUniformMatrix4fv(mvploc, 1, GL_FALSE, &mvp[0][0]);

  GLuint camloc = glGetUniformLocation(shader_programme, "cameraloc");
  glUniform3fv(camloc, 1, &camera[0]);
  glm::vec3 lightdir = glm::normalize(camera - origin); // Direction of light
  GLuint ldirloc = glGetUniformLocation(shader_programme, "lightdir");
  glUniform3fv(ldirloc, 1, &lightdir[0]);
  glm::vec4 lightcoeff(0.3, 0.7, 2.8, 50.5); // Lighting coeff, Ka, Kd, Ks, alpha
  GLuint lcoeloc = glGetUniformLocation(shader_programme, "lightcoeff");
  glUniform4fv(lcoeloc, 1, &lightcoeff[0]);

  int counter = 0;
  float phi = 0.0;
  float theta = 0.0;
  float distance = 50.0;
  float max_phi = 2.0 * glm::pi<float>();
  float max_theta = 2.0 * glm::pi<float>();
  ModelGLState modelState;

  const float frameSpeed = 1.0 / 60.0;
  const float updateSpeed = 1.0 / 60.0;
  float lastUpdateTime = 0.0;
  float lastFrameTime = 0.0;

  float inc_phi = 0 * (max_phi * updateSpeed) / 10.0;
  float inc_theta = (max_phi * updateSpeed) / 10.0;
  inc_theta *= 1.5;
  while (!glfwWindowShouldClose(window))
  {
    float now = glfwGetTime();
    float deltaUpdateTime = now - lastUpdateTime;
    float deltaFrameTime = now - lastFrameTime;

    if (deltaUpdateTime >= updateSpeed)
    {
      vao = LoadAneurysmModel(counter, num_tris, modelState);
      counter = (counter + 1) % 200;

      camera = glm::vec3(
          distance * cos(phi) * sin(theta),
          distance * sin(phi) * sin(theta),
          distance * cos(theta));
      View = glm::lookAt(
          camera, // Camera in world space
          origin, // looks at the origin
          up      // and the head is up
      );
      mvp = Projection * View * Model;

      GLuint mvploc = glGetUniformLocation(shader_programme, "MVP");
      glUniformMatrix4fv(mvploc, 1, GL_FALSE, &mvp[0][0]);
      GLuint camloc = glGetUniformLocation(shader_programme, "cameraloc");
      glUniform3fv(camloc, 1, &camera[0]);
      glm::vec3 lightdir = glm::normalize(camera - origin); // Direction of light
      GLuint ldirloc = glGetUniformLocation(shader_programme, "lightdir");
      glUniform3fv(ldirloc, 1, &lightdir[0]);

      phi = fmod(phi + inc_phi, max_phi);
      theta = fmod(theta + inc_theta, max_theta);

      lastUpdateTime = glfwGetTime();
    }

    // update other events like input handling
    glfwPollEvents();

    if (deltaFrameTime >= frameSpeed)
    {
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      glBindVertexArray(vao);
      // Draw triangles
      glDrawElements(GL_TRIANGLES, 3 * num_tris, GL_UNSIGNED_INT, NULL);
      // put the stuff we've been drawing onto the display
      glfwSwapBuffers(window);
      lastFrameTime = glfwGetTime();
    }
  }

  // close GL context and any other GLFW resources
  glfwTerminate();
  return 0;
}
