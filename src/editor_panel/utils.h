#pragma once

#include <iostream>
#include "../text_area/text_area.h"
#include "../text_area/cursor.h"
#include "raylib.h"
#include "tinyfiledialogs.h"

enum class EditorStates {OPEN, SAVE, WRITING};

struct Button {
  std::string name;
  Texture2D texture;
  Rectangle rect;
};

class Utils {
  private:
    std::vector<std::string> functionalities = {"save", "save_as", "undo", "redo", "copy", "paste"};
    std::vector<Button> buttons;

    Font font_text_tool_panel;
    std::string path_current_file_saved;
  public:
    Utils() {
      font_text_tool_panel = LoadFont("assets/font/arial.ttf");
      SetTextureFilter(font_text_tool_panel.texture, TEXTURE_FILTER_BILINEAR);

      InitToolbar();
    }
    
    void InitToolbar();
    void handleInteractions(PieceTable& text_storage, Cursor& cursor);
    void renderToolPanel();

    std::string openFileDialog();
};