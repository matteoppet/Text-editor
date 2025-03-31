#include "font.h"

FontClass& FontClass::getInstance() {
  static FontClass instance;
  return instance;
}

Vector2 FontClass::measure_size_char(char char_pressed) {
  char text[2] = {char_pressed, '\0'};
  Vector2 measure_text = MeasureTextEx(FONT_TYPE, text, FONT_SIZE, FONT_SPACING);
  return measure_text;
}

void FontClass::unloadFont() {
  UnloadFont(FONT_TYPE);
}
