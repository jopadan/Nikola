#pragma once

#include "nikola/nikola_gfx.h"

inline nikola::GfxShaderDesc generate_blinn_phong_shader() {
  return nikola::GfxShaderDesc {
    "#version 460 core"
    "\n"
    "layout (location = 0) in vec3 aPos;"
    "layout (location = 1) in vec3 aNormal;"
    "layout (location = 2) in vec2 aTextureCoords;"
    "\n"
    "out VS_OUT {"
    "  vec2 tex_coords;"
    "  vec3 normal;"
    "  vec3 pixel_pos;"
    "} vs_out;"
    "\n"
    "layout (std140, binding = 0) uniform Matrices {"
    "  mat4 u_view;"
    "  mat4 u_projection;"
    "};"
    "\n"
    "uniform mat4 u_model;"
    "uniform vec2 u_screen_size;"
    "\n"
    "void main() {"
    "  vec4 model_space = u_model * vec4(aPos, 1.0f);"
    "\n"
    "  vs_out.tex_coords = aTextureCoords;"
    "  vs_out.normal     = mat3(transpose(inverse(u_model))) * aNormal; "
    "  vs_out.pixel_pos  = vec3(model_space);"
    "\n"
    "  gl_Position       = u_projection * u_view * model_space;"
    "}",

    "#version 460 core"
    "\n"
    "layout (location = 0) out vec4 frag_color;"
    "\n"
    "in VS_OUT {"
    "  vec2 tex_coords;"
    "  vec3 normal;" 
    "  vec3 pixel_pos;"
    "} fs_in;"
    "\n"
    "#define POINT_LIGHTS_MAX 32"
    "\n"
    "struct Material {"
    "  sampler2D diffuse_map;"
    "  sampler2D specular_map;"
    "\n"
    "  vec4 color;"
    "  float shininess;"
    "};"
    "\n"
    "struct DirectionalLight {"
    "  vec3 direction;" 
    "  vec3 color;" 
    "};"
    "\n"
    "struct PointLight {"
    "  vec3 position;" 
    "  vec3 color;" 
    "\n"
    "  float linear;"
    "  float quadratic;"
    "};"
    "\n"
    "uniform Material u_material;"
    "\n"
    "uniform DirectionalLight u_dir_light;"
    "uniform PointLight u_point_lights[POINT_LIGHTS_MAX];"
    "uniform int u_point_lights_count;"
    "\n"
    "uniform vec3 u_view_pos;"
    "uniform vec3 u_ambient;"
    "\n"
    "vec3 directional_light(DirectionalLight light, vec4 diffuse_texel, vec4 specular_texel);"
    "vec3 point_light(PointLight light, vec4 diffuse_texel, vec4 specular_texel);"
    "vec3 spot_light();"
    "\n"
    "void main() {"
    "  vec4 diffuse  = texture(u_material.diffuse_map, fs_in.tex_coords) * u_material.color;"
    "  vec4 specular = texture(u_material.specular_map, fs_in.tex_coords);"
    "\n"
    "  vec3 point_lights_factor = vec3(0.0f);"
    "  for(int i = 0; i < u_point_lights_count; i++) {\n"
    "     point_lights_factor += point_light(u_point_lights[i], diffuse, specular);"
    "  }"
    "\n"
    "  vec3 dir_light_factor = directional_light(u_dir_light, diffuse, specular);"
    "\n"
    "  frag_color = vec4(vec3(point_lights_factor + dir_light_factor), 1.0);"
    "}"
    "\n"
    "vec3 directional_light(DirectionalLight light, vec4 diffuse_texel, vec4 specular_texel) {"
    "  vec3 ambient = u_ambient * vec3(diffuse_texel);"
    "\n"
    "  vec3 norm    = normalize(fs_in.normal);"
    "  float diff   = max(dot(norm, normalize(-light.direction)), 0.0);"
    "  vec3 diffuse = light.color * diff * vec3(diffuse_texel);"
    "\n"
    "  vec3 view_dir    = normalize(u_view_pos - fs_in.pixel_pos);"
    "  vec3 reflect_dir = reflect(-light.direction, norm);"
    "  float spec       = pow(max(dot(view_dir, reflect_dir), 0.0), u_material.shininess);"
    "  vec3 specular    = light.color * spec * vec3(specular_texel);"
    "\n"
    "  return ambient + diffuse;"
    "}"
    "\n"
    "vec3 point_light(PointLight light, vec4 diffuse_texel, vec4 specular_texel) {"
    "  vec3 ambient = u_ambient * vec3(diffuse_texel);"
    "\n"
    "  vec3 norm      = normalize(fs_in.normal);"
    "  vec3 light_dir = normalize(light.position - fs_in.pixel_pos);"
    "  float diff     = max(dot(norm, light_dir), 0.0);"
    "  vec3 diffuse   = diff * vec3(diffuse_texel) * light.color * diff;"
    "\n"
    "  // Specular\n" 
    "  vec3 view_dir    = normalize(u_view_pos - fs_in.pixel_pos);"
    "  vec3 reflect_dir = reflect(-light_dir, norm);"
    "  float spec       = pow(max(dot(view_dir, reflect_dir), 0.0), u_material.shininess);"
    "  vec3 specular    = light.color * spec * vec3(specular_texel);"
    "\n"
    "  float distance = length(light.position - fs_in.pixel_pos);"
    "  float atten    = 1.0 / (1.0 + light.linear * distance + light.quadratic * (distance * distance));"
    "\n"
    "  ambient *= atten;"
    "  diffuse *= atten;" 
    "  specular *= atten;"
    "\n"
    "  return ambient + diffuse;"
    "}"
    "\n"
    "vec3 spot_light() {"
    "  return vec3(0.0);"
    "}"
  };
};
