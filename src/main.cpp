// This example is heavily based on the tutorial at https://open.gl

// OpenGL Helpers to reduce the clutter
#include "Helpers.h"
#include "OBJ_Loader.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <iostream>
#include <fstream>

// GLFW is necessary to handle the OpenGL context
#include <GLFW/glfw3.h>

// Timer
#include <chrono>

#define BACKGROUND_R 0.0
#define BACKGROUND_G 0.0
#define BACKGROUND_B 0.0

#define DEFAULT_COLOR_R 1.0
#define DEFAULT_COLOR_G 1.0
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
vector<VertexBufferObject> VBO_T;   // vertex texture coords
// ElementBufferObject wrappers
vector<ElementBufferObject> EBO;

vector<Mesh> meshes;          // list to store all model meshes
vector<Object> objects;       // list to store all objects in the scene

vector<unsigned> textures;    // list to store texture IDs

Camera camera;

int highlighted = NO_HIGHLIGHTED;
bool blinkHighlight = true;

// Function to read a ".off" mesh data file
int readMesh(string filename, vector<Mesh>& meshes) {
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
        return 0;
    } catch (...) {
        std::cerr << "Error opening file." << std::endl;
        return -1;
    }
}

// read a .obj file
int readObj(string objFilename, vector<Mesh>& meshes) {
    try {
        objl::Loader loader;
        std::ifstream testFile((dataPath + objFilename).c_str());
        if (testFile.good()) {     
            loader.LoadFile((char*)(dataPath + objFilename).c_str());
        } else {
            testFile.close();
            testFile.open(("../" + dataPath + objFilename).c_str());
            if (!testFile.good()) throw 1;
            loader.LoadFile((char*)("../" + dataPath + objFilename).c_str());
        }
        testFile.close();


        unsigned int nV, nF;
        nV = loader.LoadedVertices.size();
        nF = loader.LoadedMeshes[0].Indices.size() / 3;
        Mesh mesh;
        // read vertices
        for (unsigned i = 0; i < nV; ++i) {
            Point vertex;
            vertex.x = loader.LoadedVertices[i].Position.X;
            vertex.y = loader.LoadedVertices[i].Position.Y;
            vertex.z = loader.LoadedVertices[i].Position.Z;
            mesh.V.push_back(vertex);
        }
        // read faces
        for (unsigned i = 0; i < nF; ++i) {
            Face face;
            face.a = loader.LoadedMeshes[0].Indices[i * 3];
            face.b = loader.LoadedMeshes[0].Indices[i * 3 + 1];
            face.c = loader.LoadedMeshes[0].Indices[i * 3 + 2];
            mesh.F.push_back(face);
        }
        // read vertex normals
        for (unsigned i = 0; i < nV; ++i) {
            Point normal;
            normal.x = loader.LoadedVertices[i].Normal.X;
            normal.y = loader.LoadedVertices[i].Normal.Y;
            normal.z = loader.LoadedVertices[i].Normal.Z;
            mesh.VN.push_back(normal);
        }
        // read vertex texture coordinates
        for (unsigned i = 0; i < nV; ++i) {
            Point2d texCoords;
            texCoords.u = loader.LoadedVertices[i].TextureCoordinate.X;
            texCoords.v = loader.LoadedVertices[i].TextureCoordinate.Y;
            mesh.texCorrds.push_back(texCoords);
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
        return 0;
    } catch (...) {
        std::cerr << "Error opening file." << std::endl;
        return -1;
    }
}

// read a texture picture and associate it to a mesh
int readTexture(string textureFilename, Mesh& mesh) {
    // read image file
    int width, height, nrChannels;
    unsigned char *data;
    objl::Loader loader;
    std::ifstream testFile((dataPath + textureFilename).c_str());
    if (testFile.good()) {     
        data = stbi_load((dataPath + textureFilename).c_str(), &width, &height, &nrChannels, 0);
    } else {
        testFile.close();
        testFile.open(("../" + dataPath + textureFilename).c_str());
        if (!testFile.good()) throw 1;
        data = stbi_load(("../" + dataPath + textureFilename).c_str(), &width, &height, &nrChannels, 0);
    }
    testFile.close();

    if (data) {
        unsigned int texture;
        // create a new texture
        glGenTextures(1, &texture);
        check_gl_error();
        glBindTexture(GL_TEXTURE_2D, texture);
        check_gl_error();
        // set the texture wrapping/filtering options (on the currently bound texture object)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        check_gl_error();
        // upload texture image
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        check_gl_error();
        glGenerateMipmap(GL_TEXTURE_2D);
        check_gl_error();

        stbi_image_free(data);

        // Save the texture ID for later reference
        textures.push_back(texture);
        mesh.texture = textures.size() - 1;
        return 0;
    }
    else {
        std::cout << "Failed to load texture" << std::endl;
        return -1;
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
        case  GLFW_KEY_4:
            objects.push_back(Object(3));
            break;
        case  GLFW_KEY_5:
            objects.push_back(Object(4));
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
    program.bindVertexAttribArray("texCoords",VBO_T[objects[i].model]);

    EBO[objects[i].model].bind();

    // Set textures to use in texture units
    glActiveTexture(GL_TEXTURE0);           // GL_TEXTURE0 denotes the default texture unit
    glBindTexture(GL_TEXTURE_2D, textures[meshes[objects[i].model].texture]);
    // Bind texture units to samplers
    glUniform1i(program.uniform("tex"), 0); // note to self: the parameter to bind to the sampler uniform is 0,
                                            // not GL_TEXTURE0 (which is not 0)!

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
    window = glfwCreateWindow(RESOLUTION_X, RESOLUTION_Y, "3D physics simulation", NULL, NULL);
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

    // read models from .off meshes
    readMesh("unit_cube.off", meshes);
    readMesh("bumpy_cube.off", meshes);
    readMesh("bunny.off", meshes);

    // read models from .obj files (as well as their texture pictures)
    readObj("Earth.obj", meshes);
    //readObj("fancy_sphere_1_reduced.obj", meshes);

    // read textures
    readTexture("Earth.png", meshes[3]);
    //readTexture("one_pixel_0_0.5_1.bmp", meshes[4]);

    // For each model mesh, create and initialize a VBO and EBO with its vertices data
    for (unsigned i = 0; i < meshes.size(); ++i) {
        VBO.push_back(VertexBufferObject());
        VBO[i].init();
        VBO_N.push_back(VertexBufferObject());
        VBO_N[i].init();
        VBO_T.push_back(VertexBufferObject()); // even if a mesh model doesn't have texture,
        VBO_T[i].init();                       // a position is still reserved
        EBO.push_back(ElementBufferObject());
        EBO[i].init();

        VBO[i].update(meshes[i].V);
        VBO_N[i].update(meshes[i].VN);
        VBO_T[i].update(meshes[i].texCorrds);
        EBO[i].update(meshes[i].F);
    }
    
    // declare shaders
    extern const GLchar *vertex_shader,
                        *fragment_shader_flat,
                        *fragment_shader_phong,
                        *fragment_shader_debug_normal,
                        *fragment_shader_rawColor;
    // Initialize the OpenGL Program
    // A program controls the OpenGL pipeline and it must contains
    // at least a vertex shader and a fragment shader to be valid
    Program program_flat,
            program_phong, 
            program_rawColor, 
            program_debug_normal;
    // Compile the shaders and upload the binary to the GPU
    // Note that we have to explicitly specify that the output "slot" called outColor
    // is the one that we want in the fragment buffer (and thus on screen)
    program_flat.init(vertex_shader,fragment_shader_flat,"outColor");
    program_phong.init(vertex_shader,fragment_shader_phong,"outColor");
    program_rawColor.init(vertex_shader,fragment_shader_rawColor,"outColor");
    program_debug_normal.init(vertex_shader,fragment_shader_debug_normal,"outColor");

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
        glClearColor(BACKGROUND_R, BACKGROUND_G, BACKGROUND_B, 1.0f);
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
            /*if (objects[i].wireframe) {
                myProgramInit(program_rawColor, i, time);
                glUniform3f(program_rawColor.uniform("color"), 0.0, 0.0, 0.0);
                for (unsigned j = 0; j < EBO[objects[i].model].cols; ++j) {
                    glDrawElements(GL_LINE_STRIP,
                                   EBO[objects[i].model].rows,
                                   GL_UNSIGNED_INT,
                                   (GLvoid *) (j * EBO[objects[i].model].rows * sizeof(unsigned)));
                }
            }*/
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
        VBO_T[i].free();
    }
    for (unsigned i = 0; i < EBO.size(); ++i) {
        EBO[i].free();
    }
    // Deallocate glfw internals
    glfwTerminate();
    return 0;
}
