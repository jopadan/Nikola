#include "app.h"

#include <nikola/nikola_app.h>

int main(int argc, char** argv) {
  int win_flags = nikola::WINDOW_FLAGS_FOCUS_ON_CREATE | 
                  nikola::WINDOW_FLAGS_CENTER_MOUSE    |
                  nikola::WINDOW_FLAGS_HIDE_CURSOR;

  nikola::AppDesc app_desc {
    .init_fn     = app_init,
    .shutdown_fn = app_shutdown,
    .update_fn   = app_update, 
    
    .render_fn     = app_render, 
    .render_gui_fn = app_render_gui, 

    .window_title  = "Font Testbed", 
    .window_width  = 1600, 
    .window_height = 900, 
    .window_flags  = win_flags,

    .args_values = argv, 
    .args_count  = argc,
  };

  nikola::engine_init(app_desc);
  nikola::engine_run();
  nikola::engine_shutdown();

  return 0;
}
