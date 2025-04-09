#include "cursor.h"

void Cursor::render(int start_position_x, int start_position_y) {
  float start_y = start_position_y + (current_row * size.y);
  float start_x = start_position_x + (current_col * FontClass::getInstance().measure_size_char('a').x);

  float end_y = start_y + size.y;
  float end_x = start_x + size.x;

  DrawLine(start_x, start_y, end_x, end_y, RED);
}

void Cursor::reset() {
  current_row = 0;
  current_col = 0;
  current_pos = 0;
}