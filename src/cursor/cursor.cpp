#include "cursor.h"

void Cursor::render() {
  float start_y = current_row*size.y;
  float end_y = start_y+size.y;
  float start_x = current_col*FontClass::getInstance().measure_size_char('a').x;
  float end_x = start_x+size.x;

  DrawLine(start_x, start_y, end_x, end_y, RED);
}