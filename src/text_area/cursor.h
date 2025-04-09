#pragma once

#include "raylib.h"
#include "../font/font.h"

class Cursor {
  private:
    Vector2 size = {1, 18};
  public:
    int current_row = 0;
    int current_col = 0;
    int current_pos = 0;
    bool cursor_moved = true;

    void render(int start_position_x, int start_position_y);
    void reset();
};