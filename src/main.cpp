// This example is heavily based on the tutorial at https://open.gl

// OpenGL Helpers to reduce the clutter
#include "Helpers.h"
#include <iostream>
#include <fstream>

// GLFW is necessary to handle the OpenGL context
#include <GLFW/glfw3.h>

// Timer
#include <chrono>

#define DEFAULT_COLOR_R 0.0
#define DEFAULT_COLOR_G 0.5
#define DEFAULT_COLOR_B 1.0

#define AMBIENT_COEF    0.3

#define CAMERA_PAN_FRAME_STEP   4E-3
#define CAMERA_ZOOM_FRAME_STEP  3E-3
#define TRANSLATE_FRAME_STEP    1E-2
#define ROTATE_FRAME_STEP       3E-3
#define SCALE_FRAME_STEP        3E-3

const string dataPath = "./data/";  // path for data files

#define NO_HIGHLIGHTED -1

// VertexBufferObject wrappers
vector<VertexBufferObject> VBO;     // vertex coords
vector<VertexBufferObject> VBO_N;   // vertex normals
// ElementBufferObject wrappers
vector<ElementBufferObject> EBO;

vector<Mesh> meshes;          // list to store all model meshes
vector<Object> objects;       // list to store all objects in the scene

Camera camera;

int highlighted = NO_HIGHLIGHTED;
bool blinkHighlight = true;

// Function to read a mesh data file
void readMesh(string filename, vector<Mesh>& meshes) {
    try {
        std::ifstream meshFile((dataPath + filename).c_str());
        if (!meshFile.good()) {
            meshFile.close();
            meshFile.open(("../" + dataPath + filename).c_str());
        }
        if (!meshFile.good()) throw 1;

        // Check first line
        string firstLine;
        meshFile >> firstLine;
        if (firstLine != "OFF") throw 1;

        unsigned int nV, nF, nE;
        meshFile >> nV >> nF >> nE;
        Mesh mesh;
        // read vertices
        for (unsigned i = 0; i < nV; ++i) {
            Point vertex;
            meshFile >> vertex.x >> vertex.y >> vertex.z;
            mesh.V.push_back(vertex);
        }
        // read faces
        for (unsigned i = 0; i < nF; ++i) {
            unsigned n;
            meshFile >> n;
            Face face;
            meshFile >> face.a >> face.b >> face.c;
            mesh.F.push_back(face);
        }
        // calculate face normals
        for (unsigned i = 0; i < nF; ++i) {
            Vector3f edge1, edge2;
            edge1 = Vector3f(mesh.V[mesh.F[i].b].x - mesh.V[mesh.F[i].a].x,
                             mesh.V[mesh.F[i].b].y - mesh.V[mesh.F[i].a].y,
                             mesh.V[mesh.F[i].b].z - mesh.V[mesh.F[i].a].z);
            edge2 = Vector3f(mesh.V[mesh.F[i].c].x - mesh.V[mesh.F[i].a].x,
                             mesh.V[mesh.F[i].c].y - mesh.V[mesh.F[i].a].y,
                             mesh.V[mesh.F[i].c].z - mesh.V[mesh.F[i].a].z);
            Vector3f faceNormal = edge1.cross(edge2).normalized();
            mesh.FN.push_back(Point{faceNormal.x(), faceNormal.y(), faceNormal.z()});
        }
        // calculate vertex normals
        for (unsigned i = 0; i < nV; ++i) {
            unsigned adjCount = 0;
            float sumX = 0, sumY = 0, sumZ = 0;
            for (unsigned j = 0; j < nF; ++j) {
                if (mesh.F[j].a == i || mesh.F[j].b == i || mesh.F[j].c == i) {
                    sumX += mesh.FN[j].x;
                    sumY += mesh.FN[j].y;
                    sumZ += mesh.FN[j].z;
                    ++adjCount;
                }
            }
            Vector3f vertexNormal = Vector3f(sumX / adjCount, sumY / adjCount, sumZ / adjCount).normalized();
            mesh.VN.push_back(Point{vertexNormal.x(), vertexNormal.y(), vertexNormal.z()});
        }
        // calculate barycenter
        float sumX = 0, sumY = 0, sumZ = 0;
        for (unsigned i = 0; i < nV; ++i) {
            sumX += mesh.V[i].x;
            sumY += mesh.V[i].y;
            sumZ += mesh.V[i].z;
        }
        mesh.barycenterX = sumX / nV;
        mesh.barycenterY = sumY / nV;
        mesh.barycenterZ = sumZ / nV;

        meshes.push_back(mesh);
    } catch (...) {
        std::cerr << "Error opening file." << std::endl;
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    // Get the position of the mouse in the window
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    // Get the size of the window
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    // Convert screen position to world coordinates
    double xworld = ((xpos/double(width))*2)-1;
    double yworld = (((height-1-ypos)/double(height))*2)-1; // NOTE: y axis is flipped in glfw
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        switch (key) {
        case  GLFW_KEY_1:
            objects.push_back(Object(0));
            break;
        case GLFW_KEY_2:
            objects.push_back(Object(1));
            break;
        case  GLFW_KEY_3:
            objects.push_back(Object(2));
            break;
        case GLFW_KEY_F1:
            if (highlighted != NO_HIGHLIGHTED) {
                objects[highlighted].shading = WIREFRAME;
                objects[highlighted].wireframe = true;
            }
            break;
        case GLFW_KEY_F2:
            if (highlighted != NO_HIGHLIGHTED) {
                objects[highlighted].shading = FLAT;
                objects[highlighted].wireframe = true;
            }
            break;
        case GLFW_KEY_F3:
            if (highlighted != NO_HIGHLIGHTED) {
                objects[highlighted].shading = PHONG;
                objects[highlighted].wireframe = false;
            }
            break;
        case GLFW_KEY_F4:
            if (highlighted != NO_HIGHLIGHTED) {
                objects[highlighted].shading = DEBUG_NORMAL;
            }
            break;
        case GLFW_KEY_TAB:
            if (highlighted != NO_HIGHLIGHTED) {
                objects[highlighted].wireframe = !objects[highlighted].wireframe;
            }
            break;
        case GLFW_KEY_GRAVE_ACCENT:
            if (highlighted != NO_HIGHLIGHTED) {
                blinkHighlight = !blinkHighlight;
            }
            break;
        case GLFW_KEY_ESCAPE:
            highlighted = NO_HIGHLIGHTED;
            break;
        case GLFW_KEY_SPACE:
            camera.perspective = !camera.perspective;
            break;
        case GLFW_KEY_EQUAL:
            camera.ortho_width *= 1.2;
            camera.persp_FOVx *= 1.2;
            camera.update_projection_matrix();
            break;
        case GLFW_KEY_MINUS:
            camera.ortho_width *= 0.8;
            camera.persp_FOVx *= 0.8;
            camera.update_projection_matrix();
            break;
        case GLFW_KEY_E:
            if (objects.size()) {
                ++highlighted;
                if (highlighted == objects.size()) highlighted = 0;
            }
            break;
        case GLFW_KEY_Q:
            if (objects.size()) {
                --highlighted;
                if (highlighted == -1) highlighted = objects.size() - 1;
            }
            break;
        default:
            break;
        }
    }
}

// test for key state (for holding keys)
void testKeyStates(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_W)) {
        // camera up
        camera.update_phi(camera.phi + CAMERA_PAN_FRAME_STEP);
    }
    if (glfwGetKey(window, GLFW_KEY_S)) {
        // camera down
        camera.update_phi(camera.phi - CAMERA_PAN_FRAME_STEP);
    }
    if (glfwGetKey(window, GLFW_KEY_D)) {
        // camera right
        camera.update_theta(camera.theta + CAMERA_PAN_FRAME_STEP);
    }
    if (glfwGetKey(window, GLFW_KEY_A)) {
        // camera left
        camera.update_theta(camera.theta - CAMERA_PAN_FRAME_STEP);
    }
    if (glfwGetKey(window, GLFW_KEY_F)) {
        // camera forward
        camera.update_r(camera.r - CAMERA_ZOOM_FRAME_STEP);
    }
    if (glfwGetKey(window, GLFW_KEY_V)) {
        // camera backward
        camera.update_r(camera.r + CAMERA_ZOOM_FRAME_STEP);
    }
    if (highlighted != NO_HIGHLIGHTED) {
        if (glfwGetKey(window, GLFW_KEY_I)) {
            // object translate backward
            objects[highlighted].translateZ += TRANSLATE_FRAME_STEP;
        }
        if (glfwGetKey(window, GLFW_KEY_U)) {
            // object translate forward
            objects[highlighted].translateZ -= TRANSLATE_FRAME_STEP;
        }
        if (glfwGetKey(window, GLFW_KEY_L)) {
            // object translate right
            objects[highlighted].translateX += TRANSLATE_FRAME_STEP;
        }
        if (glfwGetKey(window, GLFW_KEY_H)) {
            // object translate left
            objects[highlighted].translateX -= TRANSLATE_FRAME_STEP;
        }
        if (glfwGetKey(window, GLFW_KEY_K)) {
            // object translate up
            objects[highlighted].translateY += TRANSLATE_FRAME_STEP;
        }
        if (glfwGetKey(window, GLFW_KEY_J)) {
            // object translate down
            objects[highlighted].translateY -= TRANSLATE_FRAME_STEP;
        }
        if (glfwGetKey(window, GLFW_KEY_O)) {
            // object scale up
            objects[highlighted].scale *= 1 + SCALE_FRAME_STEP;
        }
        if (glfwGetKey(window, GLFW_KEY_P)) {
            // object scale down
            objects[highlighted].scale *= 1 - SCALE_FRAME_STEP;
        }
        if (glfwGetKey(window, GLFW_KEY_SLASH)) {
            // object rotate X 
            objects[highlighted].rotateX += ROTATE_FRAME_STEP;
        }
        if (glfwGetKey(window, GLFW_KEY_SEMICOLON)) {
            // object rotate X 
            objects[highlighted].rotateX -= ROTATE_FRAME_STEP;
        }
        if (glfwGetKey(window, GLFW_KEY_N)) {
            // object rotate Y 
            objects[highlighted].rotateY += ROTATE_FRAME_STEP;
        }
        if (glfwGetKey(window, GLFW_KEY_M)) {
            // object rotate Y 
            objects[highlighted].rotateY -= ROTATE_FRAME_STEP;
        }
        if (glfwGetKey(window, GLFW_KEY_COMMA)) {
            // object rotate Z 
            objects[highlighted].rotateZ += ROTATE_FRAME_STEP;
        }
        if (glfwGetKey(window, GLFW_KEY_PERIOD)) {
            // object rotate Z 
            objects[highlighted].rotateZ -= ROTATE_FRAME_STEP;
        }
    }
}

// Prepare a render program to use
void myProgramInit(Program& program, unsigned i, float time) {
    // specify program to use
    program.bind();
    // The vertex shader wants the position of the vertices as an input.
    // The following line connects the VBO we defined above with the position "slot"
    // in the vertex shader
    program.bindVertexAttribArray("position_m",VBO[objects[i].model]);
    program.bindVertexAttribArray("normal_m",VBO_N[objects[i].model]);
    EBO[objects[i].model].bind();

    // Set the transformation parameters for the object
    glUniform3f(program.uniform("TR"), objects[i].translateX, objects[i].translateY, objects[i].translateZ);
    glUniform3f(program.uniform("RO"), objects[i].rotateX, objects[i].rotateY, objects[i].rotateZ);
    glUniform1f(program.uniform("SC"), objects[i].scale);
    glUniform3f(program.uniform("barycenter"), meshes[objects[i].model].barycenterX, 
                meshes[objects[i].model].barycenterY, meshes[objects[i].model].barycenterZ);

    // Set the rendering parameters for the object
    glUniform1f(program.uniform("ambient_coef"), AMBIENT_COEF);
    glUniform1f(program.uniform("diffuse_coef"), objects[i].diffuse);
    glUniform1f(program.uniform("specular_coef"), objects[i].specular);
    glUniform1f(program.uniform("phongExp"), objects[i].phongExp);
    if (i == highlighted) {
        if (blinkHighlight) glUniform3f(program.uniform("color"), 0.7f, 0.7f, (sin(time * 8.0f) + 1.0f) / 2.0f);
        else glUniform3f(program.uniform("color"), 0.7f, 0.7f, 0.0f);
    } else {
        glUniform3f(program.uniform("color"), DEFAULT_COLOR_R, DEFAULT_COLOR_G, DEFAULT_COLOR_B);
    }

    // Set camera parameters (for calculating lighting)
    glUniform3f(program.uniform("camera_pos"), 
                camera.getXYZ().x(), camera.getXYZ().y(), camera.getXYZ().z());
    // Set camera view and projection matrices
    glUniformMatrix4fv(program.uniform("M_view"), 1, GL_FALSE, camera.M_view.data());
    if (camera.perspective)
        glUniformMatrix4fv(program.uniform("M_projection"), 1, GL_FALSE, camera.M_perspective.data());
    else 
        glUniformMatrix4fv(program.uniform("M_projection"), 1, GL_FALSE, camera.M_orthographic.data());
}

int main(void)
{
    GLFWwindow* window;

    // Initialize the library
    if (!glfwInit())
        return -1;

    // Activate supersampling
    glfwWindowHint(GLFW_SAMPLES, 8);

    // Ensure that we get at least a 3.2 context
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

    // On apple we have to load a core profile with forward compatibility
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Create a windowed mode window and its OpenGL context
    window = glfwCreateWindow(RESOLUTION_X, RESOLUTION_Y, "3D scene editor", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);

    #ifndef __APPLE__
      glewExperimental = true;
      GLenum err = glewInit();
      if(GLEW_OK != err)
      {
        /* Problem: glewInit failed, something is seriously wrong. */
       fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
      }
      glGetError(); // pull and savely ignonre unhandled errors like GL_INVALID_ENUM
      fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
    #endif

    int major, minor, rev;
    major = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MAJOR);
    minor = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MINOR);
    rev = glfwGetWindowAttrib(window, GLFW_CONTEXT_REVISION);
    printf("OpenGL version recieved: %d.%d.%d\n", major, minor, rev);
    printf("Supported OpenGL is %s\n", (const char*)glGetString(GL_VERSION));
    printf("Supported GLSL is %s\n", (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));

    // Register the keyboard callback
    glfwSetKeyCallback(window, key_callback);
    // Register the mouse callback
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    // Initialize the VAO
    // A Vertex Array Object (or VAO) is an object that describes how the vertex
    // attributes are stored in a Vertex Buffer Object (or VBO). This means that
    // the VAO is not the actual object storing the vertex data,
    // but the descriptor of the vertex data.
    VertexArrayObject VAO;
    VAO.init();
    VAO.bind();

    // read model meshes
    readMesh("unit_cube.off", meshes);
    readMesh("bumpy_cube.off", meshes);
    readMesh("bunny.off", meshes);
    // For each model mesh, create and initialize a VBO and EBO with its vertices data
    for (unsigned i = 0; i < meshes.size(); ++i) {
        VBO.push_back(VertexBufferObject());
        VBO[i].init();
        VBO_N.push_back(VertexBufferObject());
        VBO_N[i].init();
        EBO.push_back(ElementBufferObject());
        EBO[i].init();

        VBO[i].update(meshes[i].V);
        VBO_N[i].update(meshes[i].VN);
        EBO[i].update(meshes[i].F);
    }

    // Initialize the OpenGL Program
    // A program controls the OpenGL pipeline and it must contains
    // at least a vertex shader and a fragment shader to be valid
    Program program_flat, program_phong, program_rawColor, program_debug_normal;
    const GLchar* vertex_shader = 
        R"GLSL(
            #version 150 core
                    in vec3 position_m;
                    in vec3 normal_m;
                    out vec3 position_w;
                    out vec3 normal_w;
                    uniform vec3 TR, RO, barycenter;
                    uniform float SC;
                    uniform mat4 M_view, M_projection;

                    void main()
                    {
                        mat4 M_TR = mat4(1.0, 0.0, 0.0, 0.0,   // NOTE to self: This is the first COLUMN, not row!!!
                                         0.0, 1.0, 0.0, 0.0,
                                         0.0, 0.0, 1.0, 0.0,
                                         TR.x, TR.y, TR.z, 1.0);
                        mat4 M_RO_X = mat4(1.0, 0.0, 0.0, 0.0,
                                           0.0, cos(RO.x), sin(RO.x), 0.0,
                                           0.0, -sin(RO.x), cos(RO.x), 0.0,
                                           0.0, 0.0, 0.0, 1.0);
                        mat4 M_RO_Y = mat4(cos(RO.y), 0.0, sin(RO.y), 0.0,
                                           0.0, 1.0, 0.0, 0.0,
                                           -sin(RO.y), 0.0, cos(RO.y), 0.0,
                                           0.0, 0.0, 0.0, 1.0);
                        mat4 M_RO_Z = mat4(cos(RO.z), sin(RO.z), 0.0, 0.0,
                                           -sin(RO.z), cos(RO.z), 0.0, 0.0,
                                           0.0, 0.0, 1.0, 0.0,
                                           0.0, 0.0, 0.0, 1.0);
                        mat4 M_SC = mat4(SC, 0.0, 0.0, 0.0,
                                         0.0, SC, 0.0, 0.0,
                                         0.0, 0.0, SC, 0.0,
                                         0.0, 0.0, 0.0, 1.0);
                        mat4 M_toBarycenter = mat4(1.0, 0.0, 0.0, 0.0,
                                                   0.0, 1.0, 0.0, 0.0,
                                                   0.0, 0.0, 1.0, 0.0,
                                                   -barycenter.x, -barycenter.y, -barycenter.z, 1.0);
                        mat4 M_backFromBarycenter = mat4(1.0, 0.0, 0.0, 0.0,
                                                         0.0, 1.0, 0.0, 0.0,
                                                         0.0, 0.0, 1.0, 0.0,
                                                         barycenter.x, barycenter.y, barycenter.z, 1.0);

                        mat4 M_model = M_backFromBarycenter
                                       * M_TR * M_SC * M_RO_Z * M_RO_Y * M_RO_X
                                       * M_toBarycenter;
                        mat3 M_normal = mat3(transpose(inverse(M_model)));

                        // Calculate transformations
                        position_w = vec3(M_model * vec4(position_m, 1.0));
                        normal_w = normalize(M_normal * normal_m);
                        gl_Position = M_projection * M_view * vec4(position_w, 1.0);
                    }
        )GLSL";
    const GLchar* fragment_shader_flat = 
        R"GLSL(
            #version 150 core
                    in vec3 position_w;
                    out vec4 outColor;
                    uniform vec3 color;
                    uniform float ambient_coef, diffuse_coef, specular_coef, phongExp;
                    uniform vec3 camera_pos;

                    vec3 lightsource = vec3(5.0, 5.0, 5.0);

                    void main()
                    {
                        // calculate flat face normal
                        vec3 xTangent = dFdx(position_w);
                        vec3 yTangent = dFdy(position_w);
                        vec3 faceNormal = normalize(cross(xTangent, yTangent));

                        vec3 ambient = ambient_coef * color;
                        vec3 diffuse = diffuse_coef * color 
                                         * clamp(dot(normalize(lightsource - position_w), faceNormal), 0.0, 1.0);
                        vec3 phong = specular_coef * color
                                       * pow(clamp(dot(normalize(lightsource - position_w)
                                                       + normalize(camera_pos - position_w)
                                                   , faceNormal) / 2.0, 0.0, 1.0)
                                             , phongExp);
                        outColor = vec4(ambient + diffuse + phong, 1.0);
                    }
        )GLSL";
    const GLchar* fragment_shader_phong = 
        R"GLSL(
            #version 150 core
                    in vec3 position_w;
                    in vec3 normal_w;
                    out vec4 outColor;
                    uniform vec3 color;
                    uniform float ambient_coef, diffuse_coef, specular_coef, phongExp;
                    uniform vec3 camera_pos;

                    vec3 lightsource = vec3(5.0, 5.0, 5.0);

                    void main()
                    {
                        vec3 ambient = ambient_coef * color;
                        vec3 diffuse = diffuse_coef * color 
                                         * clamp(dot(normalize(lightsource - position_w), normal_w), 0.0, 1.0);
                        vec3 phong = specular_coef * color
                                       * pow(clamp(dot(normalize(lightsource - position_w)
                                                       + normalize(camera_pos - position_w)
                                                   , normal_w) / 2.0, 0.0, 1.0)
                                             , phongExp);
                        outColor = vec4(ambient + diffuse + phong, 1.0);
                    }
        )GLSL";
    const GLchar* fragment_shader_debug_normal = 
        R"GLSL(
            #version 150 core
                    in vec3 position_w;
                    out vec4 outColor;
                    uniform vec3 camera_pos;

                    void main()
                    {
                        vec3 xTangent = dFdx(position_w);
                        vec3 yTangent = dFdy(position_w);
                        vec3 faceNormal = normalize(cross(xTangent, yTangent));

                        outColor = vec4(faceNormal, 1.0);
                    }
        )GLSL";
    const GLchar* fragment_shader_rawColor = 
        R"GLSL(
            #version 150 core
                    out vec4 outColor;
                    uniform vec3 color;
                    void main()
                    {
                        outColor = vec4(color, 1.0);
                    }
        )GLSL";

    // Compile the two shaders and upload the binary to the GPU
    // Note that we have to explicitly specify that the output "slot" called outColor
    // is the one that we want in the fragment buffer (and thus on screen)
    program_flat.init(vertex_shader,fragment_shader_flat,"outColor");
    program_phong.init(vertex_shader,fragment_shader_phong,"outColor");
    program_rawColor.init(vertex_shader,fragment_shader_rawColor,"outColor");
    program_debug_normal.init(vertex_shader,fragment_shader_debug_normal,"outColor");

    readMesh("unit_cube.off", meshes);
    readMesh("bumpy_cube.off", meshes);
    readMesh("bunny.off", meshes);

    // enable z-buffer
    glEnable(GL_DEPTH_TEST);

    // don't miss key state change
    glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, 1);
    glfwSetInputMode(window, GLFW_STICKY_KEYS, 1);

    // Save the current time --- it will be used to dynamically change the triangle color
    auto t_start = std::chrono::high_resolution_clock::now();

    // Loop until the user closes the window
    while (!glfwWindowShouldClose(window))
    {
        // Bind your VAO (not necessary if you have only one)
        VAO.bind();

        // Clear the framebuffer and z-buffer
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  

        // calculate time difference
        auto t_now = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration_cast<std::chrono::duration<float>>(t_now - t_start).count();

        // Draw each object in the scene
        for (unsigned i = 0; i < objects.size(); ++i) {
            bool drawTriangle;
            switch (objects[i].shading) {
            case WIREFRAME:
                drawTriangle = false;
                break;
            case FLAT:
                myProgramInit(program_flat, i, time);
                drawTriangle = true;
                break;
            case PHONG:
                myProgramInit(program_phong, i, time);
                drawTriangle = true;
                break;
            case DEBUG_NORMAL:
                myProgramInit(program_debug_normal, i, time);
                drawTriangle = true;
                break;
            default:
                break;
            }
            // Draw an object
            if (drawTriangle) {
                glDrawElements(GL_TRIANGLES,
                               EBO[objects[i].model].rows * EBO[objects[i].model].cols,
                               GL_UNSIGNED_INT,
                               0);
            }
            // Draw a wireframe
            if (objects[i].wireframe) {
                myProgramInit(program_rawColor, i, time);
                glUniform3f(program_rawColor.uniform("color"), 0.0, 0.0, 0.0);
                for (unsigned j = 0; j < EBO[objects[i].model].cols; ++j) {
                    glDrawElements(GL_LINE_STRIP,
                                   EBO[objects[i].model].rows,
                                   GL_UNSIGNED_INT,
                                   (GLvoid *) (j * EBO[objects[i].model].rows * sizeof(unsigned)));
                }
            }
        }

        // Swap front and back buffers
        glfwSwapBuffers(window);

        // Poll for and process events
        glfwPollEvents();      // for keyPress and keyRelease events
        testKeyStates(window); // for testing key state (holding keys)
    }

    // Deallocate opengl memory
    program_flat.free();
    program_phong.free();
    program_rawColor.free();
    program_debug_normal.free();
    VAO.free();
    for (unsigned i = 0; i < VBO.size(); ++i) {
        VBO[i].free();
        VBO_N[i].free();
    }
    for (unsigned i = 0; i < EBO.size(); ++i) {
        EBO[i].free();
    }
    // Deallocate glfw internals
    glfwTerminate();
    return 0;
}
