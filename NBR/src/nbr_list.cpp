#include "nbr.h"

#include <nikola/nikola.h>
#include <nbr_pch.h>

//////////////////////////////////////////////////////////////////////////

namespace nbr { // Start of nbr

/// ---------------------------------------------------------------------------------------------------------
/// *** List *** 

/// ----------------------------------------------------------------------
/// Private functions

static bool check_section_dirs(const ListSection& section) {
  if(!nikola::filesystem_exists(section.local_dir)) {
    NIKOLA_LOG_ERROR("Invalid section local directory \'%s\'", section.local_dir.c_str());
    return false;
  }
  
  // Create the output directory if it doesn't exist
  //
  // @NOTE (20/5/2025, Mohamed): The `filesystem_create_directories` function returns `true` 
  // if the directories were successfully created. Otherwise, it returns 
  // `false` if the directories already exist. 
  //
  // We check here if the function returns true (meaning, the directories did not exist and need to be created) 
  // and give the user a warning just to inform them. Comlpex, I know. Sorry about that.
  if(nikola::filesystem_create_directories(section.out_dir)) {
    NIKOLA_LOG_WARN("Invalid section output directory \'%s\'. Creating new directories.", section.out_dir.c_str());
  }

  return true;
}

static bool convert_texture(const nikola::FilePath& in_path, const nikola::FilePath& save_path) {
  nikola::NBRTexture texture; 
  nikola::NBRFile nbr; 

  if(!image_loader_load_texture(&texture, in_path)) {
    return false;
  }
   
  // Save the texture
  nikola::nbr_file_save(nbr, texture, nikola::filepath_append(save_path, nikola::filepath_filename(in_path)));

  // Unload the image
  image_loader_unload_texture(texture);
  
  NIKOLA_LOG_INFO("[NBR]: Converted texture \'%s\' to \'%s\'...", in_path.c_str(), nbr.path.c_str());
  return true;
}

static bool convert_cubemap(const nikola::FilePath& in_path, const nikola::FilePath& save_path) {
  nikola::NBRCubemap cubemap; 
  nikola::NBRFile nbr; 

  if(!image_loader_load_cubemap(&cubemap, in_path)) {
    return false;
  }
   
  // Save the cubemap
  nikola::nbr_file_save(nbr, cubemap, nikola::filepath_append(save_path, nikola::filepath_filename(in_path)));

  // Unload the image
  image_loader_unload_cubemap(cubemap);
  
  NIKOLA_LOG_INFO("[NBR]: Converted cubemap \'%s\' to \'%s\'...", in_path.c_str(), nbr.path.c_str());
  return true;
}

static bool convert_shader(const nikola::FilePath& in_path, const nikola::FilePath& save_path) {
  nikola::NBRShader shader; 
  nikola::NBRFile nbr; 

  if(!shader_loader_load(&shader, in_path)) {
    return false;
  }

  // Save the shader
  nikola::nbr_file_save(nbr, shader, nikola::filepath_append(save_path, nikola::filepath_filename(in_path)));

  // Unload the shader
  shader_loader_unload(shader);
  
  NIKOLA_LOG_INFO("[NBR]: Converted shader \'%s\' to \'%s\'...", in_path.c_str(), nbr.path.c_str());
  return true;
}

static bool convert_model(const nikola::FilePath& in_path, const nikola::FilePath& save_path) {
  nikola::NBRModel model; 
  nikola::NBRFile nbr; 

  if(!model_loader_load(&model, in_path)) {
    return false;
  }

  // Save the model
  nikola::nbr_file_save(nbr, model, nikola::filepath_append(save_path, nikola::filepath_filename(in_path)));

  // Unload the model
  model_loader_unload(model);
  
  NIKOLA_LOG_INFO("[NBR]: Converted model \'%s\' to \'%s\'...", in_path.c_str(), nbr.path.c_str());
  return true;
}

static bool convert_font(const nikola::FilePath& in_path, const nikola::FilePath& save_path) {
  nikola::NBRFont font; 
  nikola::NBRFile nbr; 

  if(!font_loader_load(&font, in_path)) {
    return false;
  }

  // Save the font
  nikola::nbr_file_save(nbr, font, nikola::filepath_append(save_path, nikola::filepath_filename(in_path)));

  // Unload the font
  font_loader_unload(font);
  
  NIKOLA_LOG_INFO("[NBR]: Converted font \'%s\' to \'%s\'...", in_path.c_str(), nbr.path.c_str());
  return true;
}

static bool convert_audio(const nikola::FilePath& in_path, const nikola::FilePath& save_path) {
  nikola::NBRAudio audio; 
  nikola::NBRFile nbr; 

  if(!audio_loader_load(&audio, in_path)) {
    return false;
  }

  // Save the audio buffer
  nikola::nbr_file_save(nbr, audio, nikola::filepath_append(save_path, nikola::filepath_filename(in_path)));

  // Unload the audio buffer
  audio_loader_unload(audio);
  
  NIKOLA_LOG_INFO("[NBR]: Converted audio \'%s\' to \'%s\'...", in_path.c_str(), nbr.path.c_str());
  return true;
}

static void convert_by_type(ListSection* section, const nikola::FilePath& path) {
  switch(section->type) {
    case nikola::RESOURCE_TYPE_TEXTURE:
      convert_texture(path, section->out_dir);
      break;
    case nikola::RESOURCE_TYPE_CUBEMAP:
      convert_cubemap(path, section->out_dir);
      break;
    case nikola::RESOURCE_TYPE_SHADER:
      convert_shader(path, section->out_dir);
      break;
    case nikola::RESOURCE_TYPE_MODEL:
      convert_model(path, section->out_dir);
      break;
    case nikola::RESOURCE_TYPE_FONT:
      convert_font(path, section->out_dir);
      break;
    case nikola::RESOURCE_TYPE_AUDIO_BUFFER:
      convert_audio(path, section->out_dir);
      break;
  }
}

static void iterate_resources(const nikola::FilePath& base_dir, const nikola::FilePath& current_path, void* user_data) {
  ListSection* section = (ListSection*)user_data;
  convert_by_type(section, current_path);
}
  
static void load_resources(ListSection* section) {
  // Check if all the paths are correct
  if(!check_section_dirs(*section)) {
    return;
  }

  // Convert all the resource paths
  for(auto& res : section->resources) {
    if(nikola::filepath_is_dir(res)) {
      nikola::filesystem_directory_iterate(res, iterate_resources, section);
      continue;
    }

    convert_by_type(section, res);
  }
}

/// Private functions
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// List context functions 

void list_context_create(const nikola::FilePath& path, ListContext* list) {
  // Lex
  nikola::DynamicArray<ListToken> tokens;
  if(!list_lexer_init(path, &tokens)) {
    return;
  }

  // Parse
  if(!list_parser_init(tokens, list)) {
    return;
  }
}

void list_context_convert_by_type(ListContext* list, const nikola::ResourceType type) {
  // @TODO (Threads): Substitute this for a thread pool/job queue 
  nikola::DynamicArray<std::thread> threads;
 
  for(auto& section : list->sections) {
    if(section.type != type) {
      continue;
    }

    // Convert all the resource paths
    threads.push_back(std::thread(load_resources, &section));
  }
  
  for(auto& th : threads) {
    th.join();
  }
}

void list_context_convert_all(ListContext* list) {
  // @TODO (Threads): Substitute this for a thread pool/job queue 
  nikola::DynamicArray<std::thread> threads;
  
  for(auto& section : list->sections) {
    // Convert all the resource paths
    threads.push_back(std::thread(load_resources, &section));
  }
  
  for(auto& th : threads) {
    th.join();
  }
}

/// List context functions 
/// ----------------------------------------------------------------------

/// *** List *** 
/// ---------------------------------------------------------------------------------------------------------


} // End of nbr

//////////////////////////////////////////////////////////////////////////
