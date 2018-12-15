#include "Helpers.h"

#include <iostream>
#include <fstream>

void Camera::update_theta(float val) {
    theta = val;
    update_view_matrix();
}

void Camera::update_phi(float val) {
    phi = val;
    update_view_matrix();
}

void Camera::update_r(float val) {
    r = val;
    update_view_matrix();
}

// Convert to orthogonal coordinates
Vector3f Camera::getXYZ() {
    return Vector3f(r * cos(phi) * sin(theta), 
                    r * sin(phi), 
                    r * cos(phi) * cos(theta));
}

void Camera::update_view_matrix() {
    // The three base vectors of the camera (view) coordinates frame
    Vector3f position = getXYZ();
    Vector3f zBase = (position - lookTarget).normalized();
    Vector3f xBase = upDirection.cross(zBase).normalized();
    Vector3f yBase = zBase.cross(xBase);
    M_view << xBase.x(), xBase.y(), xBase.z(), -xBase.dot(position),
        yBase.x(), yBase.y(), yBase.z(), -yBase.dot(position),
        zBase.x(), zBase.y(), zBase.z(), -zBase.dot(position),
        0,         0,         0,         1;
}

void Camera::update_projection_matrix() {
    M_orthographic << 1 / ortho_width, 0, 0, 0,
                      0, 1 / ortho_width * RESOLUTION_Y / RESOLUTION_X, 0, 0,
                      0, 0, -2 / (Z_far_limit - Z_near_limit), -(Z_far_limit + Z_near_limit) / (Z_far_limit - Z_near_limit),
                      0, 0, 0, 1;
    M_perspective << atan(persp_FOVx / 2), 0, 0, 0,
                     0, atan(persp_FOVx / 2 * RESOLUTION_Y / RESOLUTION_X), 0, 0,
                     0, 0, -(Z_far_limit + Z_near_limit) / (Z_far_limit - Z_near_limit), -2 * Z_far_limit * Z_near_limit / (Z_far_limit - Z_near_limit),
                     0, 0, -1, 0;
};

Camera::Camera() : theta(DEFAULT_CAMERA_THETA), phi(DEFAULT_CAMERA_PHI), r(DEFAULT_CAMERA_R), 
                   Z_far_limit(100.0), Z_near_limit(1E-3), 
                   ortho_width(DEFAULT_ORTHO_WIDTH), persp_FOVx(DEFAULT_PERSP_FOVX),
                   lookTarget(0.0, 0.0, 0.0), upDirection(0.0, 1.0, 0.0), perspective(true) {
    update_view_matrix();
    update_projection_matrix();
}

Object::Object(unsigned _model) : model(_model) {
    translateX = -meshes[_model].barycenterX;
    translateY = -meshes[_model].barycenterY;
    translateZ = -meshes[_model].barycenterZ;
    switch (_model) {
    case 0:
        scale = 1.0;
        break;
    case 1:
        scale = 0.2;
        break;
    case 2:
        scale = 10.0;
        break;
    default:
        break;
    }
}

void VertexArrayObject::init()
{
  glGenVertexArrays(1, &id);
  check_gl_error();
}

void VertexArrayObject::bind()
{
  glBindVertexArray(id);
  check_gl_error();
}

void VertexArrayObject::free()
{
  glDeleteVertexArrays(1, &id);
  check_gl_error();
}

void VertexBufferObject::init()
{
  glGenBuffers(1,&id);
  check_gl_error();
}

void VertexBufferObject::bind()
{
  glBindBuffer(GL_ARRAY_BUFFER,id);
  check_gl_error();
}

void VertexBufferObject::free()
{
  glDeleteBuffers(1,&id);
  check_gl_error();
}

void VertexBufferObject::update(const vector<Point>& coords)
{
  if (!coords.size()) return;
  assert(id != 0);
  glBindBuffer(GL_ARRAY_BUFFER, id);
  glBufferData(GL_ARRAY_BUFFER, sizeof(Point)*coords.size(), &coords[0], GL_STATIC_DRAW);
  rows = sizeof(Point) / sizeof(float);
  cols = coords.size();
  check_gl_error();
}

void VertexBufferObject::update(const vector<Point2d>& coords)
{
    if (!coords.size()) return;
    assert(id != 0);
    glBindBuffer(GL_ARRAY_BUFFER, id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Point2d)*coords.size(), &coords[0], GL_STATIC_DRAW);
    rows = sizeof(Point2d) / sizeof(float);
    cols = coords.size();
    check_gl_error();
}

void ElementBufferObject::init()
{
    glGenBuffers(1,&id);
    check_gl_error();
}

void ElementBufferObject::bind()
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,id);
    check_gl_error();
}

void ElementBufferObject::free()
{
    glDeleteBuffers(1,&id);
    check_gl_error();
}

void ElementBufferObject::update(const vector<Face>& faces)
{
    if (!faces.size()) return;
    assert(id != 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Face)*faces.size(), &faces[0], GL_STATIC_DRAW);
    rows = sizeof(Face) / sizeof(unsigned);
    cols = faces.size();
    check_gl_error();
}

bool Program::init(
  const std::string &vertex_shader_string,
  const std::string &fragment_shader_string,
  const std::string &fragment_data_name)
{
  using namespace std;
  vertex_shader = create_shader_helper(GL_VERTEX_SHADER, vertex_shader_string);
  fragment_shader = create_shader_helper(GL_FRAGMENT_SHADER, fragment_shader_string);

  if (!vertex_shader || !fragment_shader)
    return false;

  program_shader = glCreateProgram();

  glAttachShader(program_shader, vertex_shader);
  glAttachShader(program_shader, fragment_shader);

  glBindFragDataLocation(program_shader, 0, fragment_data_name.c_str());
  glLinkProgram(program_shader);

  GLint status;
  glGetProgramiv(program_shader, GL_LINK_STATUS, &status);

  if (status != GL_TRUE)
  {
    char buffer[512];
    glGetProgramInfoLog(program_shader, 512, NULL, buffer);
    cerr << "Linker error: " << endl << buffer << endl;
    program_shader = 0;
    return false;
  }

  check_gl_error();
  return true;
}

void Program::bind()
{
  glUseProgram(program_shader);
  check_gl_error();
}

GLint Program::attrib(const std::string &name) const
{
  return glGetAttribLocation(program_shader, name.c_str());
}

GLint Program::uniform(const std::string &name) const
{
  return glGetUniformLocation(program_shader, name.c_str());
}

GLint Program::bindVertexAttribArray(
        const std::string &name, VertexBufferObject& VBO) const
{
  GLint id = attrib(name);
  if (id < 0)
    return id;
  if (VBO.id == 0)
  {
    glDisableVertexAttribArray(id);
    return id;
  }
  VBO.bind();
  glEnableVertexAttribArray(id);
  glVertexAttribPointer(id, VBO.rows, GL_FLOAT, GL_FALSE, 0, 0);
  check_gl_error();

  return id;
}

void Program::free()
{
  if (program_shader)
  {
    glDeleteProgram(program_shader);
    program_shader = 0;
  }
  if (vertex_shader)
  {
    glDeleteShader(vertex_shader);
    vertex_shader = 0;
  }
  if (fragment_shader)
  {
    glDeleteShader(fragment_shader);
    fragment_shader = 0;
  }
  check_gl_error();
}

GLuint Program::create_shader_helper(GLint type, const std::string &shader_string)
{
  using namespace std;
  if (shader_string.empty())
    return (GLuint) 0;

  GLuint id = glCreateShader(type);
  const char *shader_string_const = shader_string.c_str();
  glShaderSource(id, 1, &shader_string_const, NULL);
  glCompileShader(id);

  GLint status;
  glGetShaderiv(id, GL_COMPILE_STATUS, &status);

  if (status != GL_TRUE)
  {
    char buffer[512];
    if (type == GL_VERTEX_SHADER)
      cerr << "Vertex shader:" << endl;
    else if (type == GL_FRAGMENT_SHADER)
      cerr << "Fragment shader:" << endl;
    else if (type == GL_GEOMETRY_SHADER)
      cerr << "Geometry shader:" << endl;
    cerr << shader_string << endl << endl;
    glGetShaderInfoLog(id, 512, NULL, buffer);
    cerr << "Error: " << endl << buffer << endl;
    return (GLuint) 0;
  }
  check_gl_error();

  return id;
}

void _check_gl_error(const char *file, int line)
{
  GLenum err (glGetError());

  while(err!=GL_NO_ERROR)
  {
    std::string error;

    switch(err)
    {
      case GL_INVALID_OPERATION:      error="INVALID_OPERATION";      break;
      case GL_INVALID_ENUM:           error="INVALID_ENUM";           break;
      case GL_INVALID_VALUE:          error="INVALID_VALUE";          break;
      case GL_OUT_OF_MEMORY:          error="OUT_OF_MEMORY";          break;
      case GL_INVALID_FRAMEBUFFER_OPERATION:  error="INVALID_FRAMEBUFFER_OPERATION";  break;
    }

    std::cerr << "GL_" << error.c_str() << " - " << file << ":" << line << std::endl;
    err = glGetError();
  }
}


