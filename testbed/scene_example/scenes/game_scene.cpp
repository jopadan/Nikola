#include "game_scene.h"
#include "scene_manager.h"

#include <nikola/nikola.h>
#include <imgui/imgui.h>

/// ----------------------------------------------------------------------
/// Entity
struct Entity {
  nikola::Transform transform;
  nikola::ResourceID renderable_id, material_id;
  nikola::String name;

  Entity(const nikola::Vec3& pos, const nikola::ResourceID& res_id, const nikola::ResourceID& mat_id, const char* debug_name) {
    nikola::transform_translate(transform, pos);
    nikola::transform_scale(transform, nikola::Vec3(1.0f));

    renderable_id = res_id;
    material_id   = mat_id;

    name = debug_name;
  }

  void render() {
    if(renderable_id._type == nikola::RESOURCE_TYPE_MESH) {
      nikola::renderer_queue_mesh(renderable_id, transform, material_id);
    }
    else if(renderable_id._type == nikola::RESOURCE_TYPE_MODEL) {
      nikola::renderer_queue_model(renderable_id, transform);
    }
  }

  void render_gui() {
    nikola::gui_edit_transform(name.c_str(), &transform); 
  }

  void serialize(nikola::File& file) {
    nikola::file_write_bytes(file, transform);
  }
  
  void deserialize(nikola::File& file) {
    nikola::file_read_bytes(file, &transform);
  }
};
/// Entity
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Globals

static nikola::DynamicArray<Entity> s_entities;
static nikola::f32 rotation   = 0.0f;
static const char* SCENE_PATH = "scenes/game_scene_tempel.nscn"; 

/// Globals
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Private functions

static void init_lights(Scene* scene) {
  // Directional light
  scene->frame_data.dir_light.direction = nikola::Vec3(0.0f, 0.0f, 0.0f);
  scene->frame_data.dir_light.color     = nikola::Vec3(0.0f, 0.0f, 0.0f);
 
  // Point lights
  scene->frame_data.point_lights.push_back(nikola::PointLight{nikola::Vec3(10.0f, 0.0f, 10.0f)});
  scene->frame_data.point_lights.push_back(nikola::PointLight{nikola::Vec3(10.0f, 0.0f, 10.0f)});

  // Set the ambiance
  scene->frame_data.ambient = nikola::Vec3(0.0f);
}

static nikola::f32 ease_in_sine(nikola::f32 x) {
  return 1.0f - nikola::cos((x * nikola::PI) / 2.0f);
}

/// Private functions
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// GameScene functions 

void game_scene_init() {
  SceneDesc desc =  {
    .create_func  = game_scene_create, 
    .destroy_func = game_scene_destroy, 
    .update_func  = game_scene_update, 

    .render_func     = game_scene_render, 
    .render_gui_func = game_scene_render_gui,

    .serialize_func   = game_scene_save, 
    .deserialize_func = game_scene_load,

    .user_data = nullptr,
  };

  scenes_push_scene("Game Scene", desc);
}

bool game_scene_create(Scene* scene) {
  nikola::u16 res_group = scene->resource_group;

  // Camera init
  nikola::CameraDesc cam_desc = {
    .position     = nikola::Vec3(10.0f, 0.0f, 10.0f),
    .target       = nikola::Vec3(-3.0f, 0.0f, 0.0f),
    .up_axis      = nikola::Vec3(0.0f, 1.0f, 0.0f),
    .aspect_ratio = nikola::window_get_aspect_ratio(scene->window),
    .move_func    = nikola::camera_free_move_func,
  };
  nikola::camera_create(&scene->frame_data.camera, cam_desc);
   
  // Models init
  nikola::ResourceID model = nikola::resources_push_model(res_group, "models/tempel.nbrmodel");
  // nikola::ResourceID model = nikola::resources_push_model(res_group, "models/bridge.nbrmodel");
  // nikola::ResourceID model = nikola::resources_push_model(res_group, "models/ps1.nbrmodel");

  // Textures init
  nikola::ResourceID mesh_texture = nikola::resources_push_texture(res_group, "textures/opengl.nbrtexture");
  nikola::ResourceID pav_texture  = nikola::resources_push_texture(res_group, "textures/paviment.nbrtexture");

  // Materials init
  nikola::ResourceID material_id = nikola::resources_push_material(res_group, mesh_texture);
  
  nikola::ResourceID pav_material_id = nikola::resources_push_material(res_group, pav_texture);
  
  // Meshes init
  nikola::ResourceID cube_mesh = nikola::resources_push_mesh(scene->resource_group, nikola::GEOMETRY_CUBE);

  // Skyboxes init
  scene->frame_data.skybox_id = nikola::resources_push_skybox(res_group, "cubemaps/NightSky.nbrcubemap");
  // scene->frame_data.skybox_id = {};

  // Model init
  s_entities.emplace_back(nikola::Vec3(10.0f, 0.0f, 10.0f), model, material_id, "3D Model"); 
  
  // Cube init
  s_entities.emplace_back(nikola::Vec3(10.0f, 0.0f, 10.0f), cube_mesh, material_id, "Cube"); 

  // Ground init
  s_entities.emplace_back(nikola::Vec3(10.0f, 0.0f, 10.0f), cube_mesh, pav_material_id, "Ground"); 

  // Torches 
  nikola::ResourceID torch_model = nikola::resources_push_model(res_group, "models/column_torch.nbrmodel");
  s_entities.emplace_back(nikola::Vec3(10.0f, 0.0f, 10.0f), torch_model, material_id, "Torch 0"); 
  s_entities.emplace_back(nikola::Vec3(10.0f, 0.0f, 10.0f), torch_model, material_id, "Torch 1"); 

  // Lights init
  init_lights(scene);

  // Loading the binary scene
  game_scene_load(scene, SCENE_PATH);

  return true;
}

void game_scene_destroy(Scene* scene) {
  nikola::resources_destroy_group(scene->resource_group);
}

void game_scene_update(Scene* scene, const nikola::f64 dt) {
  // Enable fullscreen
  if(nikola::input_key_pressed(nikola::KEY_F)) {
    nikola::window_set_fullscreen(scene->window, !nikola::window_is_fullscreen(scene->window));
  }
  
  // Enable editor
  if(nikola::input_key_pressed(nikola::KEY_E)) {
    scene->has_editor                  = !scene->has_editor;
    scene->frame_data.camera.is_active = !scene->has_editor;

    nikola::input_cursor_show(scene->has_editor);
  }
  
  nikola::f32 value = ease_in_sine(dt);
  rotation += value * 50.0f;
  nikola::transform_rotate(s_entities[1].transform, nikola::Vec3(1.0f), rotation);

  nikola::camera_update(scene->frame_data.camera);
}

void game_scene_render(Scene* scene) {
  // Render entities 
  for(auto& entt : s_entities) {
    entt.render();
  }
}

void game_scene_render_gui(Scene* scene) {
  if(!scene->has_editor) {
    return;
  }
  
  nikola::gui_begin_panel("Game Scene");
  
  // Entities
  if(ImGui::CollapsingHeader("Entities")) {
    for(auto& entt : s_entities) {
      entt.render_gui();
    }
  } 
   
  // Lights
  if(ImGui::CollapsingHeader("Lights")) {
    nikola::gui_edit_directional_light("Directional", &scene->frame_data.dir_light);

    for(int i = 0; i < scene->frame_data.point_lights.size(); i++) {
      nikola::PointLight* light = &scene->frame_data.point_lights[i];
      nikola::String light_name = ("Point " + std::to_string(i));

      nikola::gui_edit_point_light(light_name.c_str(), light);
    }
  
    if(ImGui::Button("Add PointLight")) {
      scene->frame_data.point_lights.push_back(nikola::PointLight{nikola::Vec3(10.0f, 0.0f, 10.0f)});
    }
  }

  // Camera
  if(ImGui::CollapsingHeader("Camera")) {
    nikola::gui_edit_camera("Editor Camera", &scene->frame_data.camera); 
    ImGui::DragFloat3("Ambient", &scene->frame_data.ambient[0], 0.1f, 0.0f, 1.0f);
  } 

  // Save button
  ImGui::NewLine(); 
  if(ImGui::Button("Save Scene")) {
    game_scene_save(scene, SCENE_PATH);
  }

  nikola::gui_end_panel();
}

void game_scene_save(Scene* scene, const nikola::FilePath& path) {
  nikola::File file;
  if(!nikola::file_open(&file, path, (int)(nikola::FILE_OPEN_WRITE | nikola::FILE_OPEN_BINARY))) {
    NIKOLA_LOG_ERROR("Cannot open Scene file at \'%s\'", path.c_str());
    return;
  }

  // Save the camera
  nikola::file_write_bytes(file, scene->frame_data.camera);

  // Save the directional light
  nikola::file_write_bytes(file, scene->frame_data.dir_light);

  // Save the point lights
  for(auto& light : scene->frame_data.point_lights) {
    nikola::file_write_bytes(file, light);
  }

  // Save the entities count
  nikola::i32 count = (nikola::i32)s_entities.size(); 
  nikola::file_write_bytes(file, &count, sizeof(nikola::i32));

  // Save the entities 
  for(auto& entt : s_entities) {
    entt.serialize(file);
  }

  nikola::file_close(file); 
}

void game_scene_load(Scene* scene, const nikola::FilePath& path) {
  nikola::File file;
  if(!nikola::file_open(&file, path, (int)(nikola::FILE_OPEN_READ | nikola::FILE_OPEN_BINARY))) {
    NIKOLA_LOG_ERROR("Cannot open Scene file at \'%s\'", path.c_str());
    return;
  }

  // Load the camera
  nikola::file_read_bytes(file, &scene->frame_data.camera);

  // Load the directional light
  nikola::file_read_bytes(file, &scene->frame_data.dir_light);

  // Load the point lights
  for(auto& light : scene->frame_data.point_lights) {
    nikola::file_read_bytes(file, &light);
  }
  
  // Load the entities count
  nikola::i32 count = 0; 
  nikola::file_read_bytes(file, &count, sizeof(nikola::i32));

  // Load the entities 
  for(nikola::sizei i = 0; i < count; i++) {
    Entity* entt = &s_entities[i];
    entt->deserialize(file);
  }

  nikola::file_close(file);
}

/// GameScene functions 
/// ----------------------------------------------------------------------
