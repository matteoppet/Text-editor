#pragma once

#include <iostream>
#include "../text/piece_table.h"
#include "../cursor/cursor.h"
#include "raylib.h"

class Utils {
  private:
    Rectangle open_file_button = {GetScreenWidth()-50, GetScreenHeight()-50, 40, 40};
  public:
    void handleInteractions(PieceTable& text_storage, Cursor& cursor);
};