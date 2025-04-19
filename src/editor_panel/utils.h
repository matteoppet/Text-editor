#pragma once

#include <iostream>
#include "../text_area/text_area.h"
#include "../text_area/cursor.h"
#include "raylib.h"
#include "tinyfiledialogs.h"

enum class EditorStates {OPEN, SAVE, WRITING};

class Utils {
  private:
    Rectangle save_file_button = {0, 0, 40, 20};
    Rectangle save_as_file_button = {40, 0, 40, 20};
    Rectangle undo_button = {95, 0, 40, 20};
    Rectangle redo_button = {135, 0, 40, 20};
    Rectangle copy_button = {175, 0, 40, 20};
    Rectangle paste_button = {215, 0, 40, 20};

    Font font_text_tool_panel;

    std::string path_current_file_saved;

  public:
    Utils() {
      font_text_tool_panel = LoadFont("assets/font/arial.ttf");
      SetTextureFilter(font_text_tool_panel.texture, TEXTURE_FILTER_BILINEAR);
    }

    void handleInteractions(PieceTable& text_storage, Cursor& cursor);
    void renderToolPanel();
    std::string openFileDialog();
};