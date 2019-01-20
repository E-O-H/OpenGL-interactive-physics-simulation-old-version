#pragma once

#include "my_openGL_helpers.h"

const GLchar* vertex_shader = 
R"GLSL(
            #version 150 core
                    in vec3 position_m;
                    in vec3 normal_m;
                    in vec2 texCoords;
                    out vec3 position_w;
                    out vec3 normal_w;
                    out vec2 texCoords_;
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
                        gl_Position = M_projection * M_view * vec4(position_w, 1.0);
                        normal_w = normalize(M_normal * normal_m);
                        texCoords_ = texCoords;
                    }
        )GLSL";

const GLchar* vertex_shader_HUD = 
R"GLSL(
            #version 150 core
                    in vec3 position_m;
                    in vec3 normal_m;
                    in vec2 texCoords;
                    out vec3 position_w;
                    out vec3 normal_w;
                    out vec2 texCoords_;
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
                        mat4 M_SC = mat4(1.0, 0.0, 0.0, 0.0,
                                         0.0, 1.0, 0.0, 0.0,
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
                        gl_Position = M_projection * vec4(position_w, 1.0);
                        normal_w = normalize(M_normal * normal_m);
                        texCoords_ = texCoords;
                    }
        )GLSL";

extern const GLchar* fragment_shader_flat = 
R"GLSL(
            #version 150 core
                    in vec3 position_w;
                    in vec2 texCoords_;
                    out vec4 outColor;
                    uniform vec3 color;
                    uniform sampler2D tex;
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
                        outColor = texture(tex, texCoords_) * vec4(ambient + diffuse + phong, 1.0);
                    }
        )GLSL";

extern const GLchar* fragment_shader_phong = 
R"GLSL(
            #version 150 core
                    in vec3 position_w;
                    in vec3 normal_w;
                    in vec2 texCoords_;
                    out vec4 outColor;
                    uniform vec3 color;
                    uniform sampler2D tex;
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
                        outColor = texture(tex, texCoords_) * vec4(ambient + diffuse + phong, 1.0);
                    }
        )GLSL";

extern const GLchar* fragment_shader_debug_normal = 
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

extern const GLchar* fragment_shader_rawColor = 
R"GLSL(
            #version 150 core
                    out vec4 outColor;
                    uniform vec3 color;
                    void main()
                    {
                        outColor = vec4(color, 1.0);
                    }
        )GLSL";