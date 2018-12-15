#ifndef HELPERS_H
#define HELPERS_H

#include <string>
#include <vector>
#include <cmath>
#include <utility>
// Linear Algebra Library
#include <Eigen/Core>
#include <Eigen/Geometry>

using std::string;
using std::vector;
using std::pair;
using std::sin;
using std::cos;
using std::atan;
using Eigen::Vector3f;
using Eigen::Vector4f;
using Eigen::Matrix4f;

#ifdef _WIN32
#  include <windows.h>
#  undef max
#  undef min
#  undef DrawText
#endif

#ifndef __APPLE__
#  define GLEW_STATIC
#  include <GL/glew.h>
#endif

#ifdef __APPLE__
#   include <OpenGL/gl3.h>
#   define __gl_h_ /* Prevent inclusion of the old gl.h */
#else
#   ifdef _WIN32
#       include <windows.h>
#   endif
#   include <GL/gl.h>
#endif

#define PI 3.1415926

#define RESOLUTION_X 1080
#define RESOLUTION_Y 1080

#define DEFAULT_CAMERA_THETA 0.0
#define DEFAULT_CAMERA_PHI   0.0
#define DEFAULT_CAMERA_R     2.0
#define DEFAULT_ORTHO_WIDTH  2.0
#define DEFAULT_PERSP_FOVX   PI / 2



class Mesh;
class Object;
extern vector<Mesh> meshes;          // list to store all model meshes
extern vector<Object> objects;       // list to store all objects in the scene

enum shading_t {
    WIREFRAME,
    FLAT,
    PHONG,
    DEBUG_NORMAL
};

struct Point {
    float x, y, z;
};

struct Point2d {
    float u, v;
};

struct Face {
    unsigned a, b, c;
};

// Class to represent a model mesh
class Mesh {
public:
    vector<Point> V;   // List of vertices
    vector<Face>  F;   // List of faces

    vector<Point> VN;   // List of vertex normals
    vector<Point> FN;   // List of face normals
    
    vector<Point2d> texCorrds; // List of vertex texture coords
    unsigned texture = -1;     // texture data ID; -1 stands for no texture

    float barycenterX, barycenterY, barycenterZ;

    Mesh() {};
};

// Class to represent an object
class Object {
public:
    unsigned model;

    shading_t shading = PHONG;
    bool wireframe = true;
    unsigned int colorR = 255;
    unsigned int colorG = 255;
    unsigned int colorB = 255;
    float alpha = 1;
    float diffuse = 0.8;
    float specular = 300;
    int phongExp = 25;

    float translateX = 0, translateY = 0, translateZ = 0;
    float rotateX = 0, rotateY = 0, rotateZ = 0;
    float scale = 1;

    Object(unsigned _model = 0);
};

// Class to represent camera
class Camera {
public:
    float theta, phi, r; 
    Vector3f lookTarget, upDirection;
    bool perspective;
    float Z_far_limit, Z_near_limit, ortho_width, persp_FOVx;

    Matrix4f M_view, M_perspective, M_orthographic;
    
    void update_theta(float val);
    void update_phi(float val);
    void update_r(float val);

    // Convert to orthogonal coordinates
    Vector3f getXYZ();

    // Updates the view matrix
    void update_view_matrix();
    // Updates the projection matrix
    void update_projection_matrix();

    Camera();
};

class VertexArrayObject
{
public:
    unsigned int id;

    VertexArrayObject() : id(0) {}

    // Create a new VAO
    void init();

    // Select this VAO for subsequent draw calls
    void bind();

    // Release the id
    void free();
};

class VertexBufferObject
{
public:
    typedef unsigned int GLuint;
    typedef int GLint;

    GLuint id;
    GLuint rows;
    GLuint cols;

    VertexBufferObject() : id(0), rows(0), cols(0) {}

    // Create a new empty VBO
    void init();

    // Updates the VBO with a mesh
    void update(const vector<Point>& coords);
    void update(const vector<Point2d>& coords);

    // Select this VBO for subsequent draw calls
    void bind();

    // Release the id
    void free();
};

class ElementBufferObject
{
public:
    typedef unsigned int GLuint;
    typedef int GLint;

    GLuint id;
    GLuint rows;
    GLuint cols;

    ElementBufferObject() : id(0), rows(0), cols(0) {}

    // Create a new empty EBO
    void init();

    // Updates the EBO with a mesh
    void update(const vector<Face>& faces);

    // Select this EBO for subsequent draw calls
    void bind();

    // Release the id
    void free();
};

// This class wraps an OpenGL program composed of two shaders
class Program
{
public:
  typedef unsigned int GLuint;
  typedef int GLint;

  GLuint vertex_shader;
  GLuint fragment_shader;
  GLuint program_shader;

  Program() : vertex_shader(0), fragment_shader(0), program_shader(0) { }

  // Create a new shader from the specified source strings
  bool init(const std::string &vertex_shader_string,
  const std::string &fragment_shader_string,
  const std::string &fragment_data_name);

  // Select this shader for subsequent draw calls
  void bind();

  // Release all OpenGL objects
  void free();

  // Return the OpenGL handle of a named shader attribute (-1 if it does not exist)
  GLint attrib(const std::string &name) const;

  // Return the OpenGL handle of a uniform attribute (-1 if it does not exist)
  GLint uniform(const std::string &name) const;

  // Bind a per-vertex array attribute
  GLint bindVertexAttribArray(const std::string &name, VertexBufferObject& VBO) const;

  GLuint create_shader_helper(GLint type, const std::string &shader_string);

};

// From: https://blog.nobel-joergensen.com/2013/01/29/debugging-opengl-using-glgeterror/
void _check_gl_error(const char *file, int line);

///
/// Usage
/// [... some opengl calls]
/// glCheckError();
///
#define check_gl_error() _check_gl_error(__FILE__,__LINE__)

#endif
