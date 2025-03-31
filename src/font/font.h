#pragma once

#include "raylib.h"
#include <iostream>
#include <vector>

class FontClass {
  public:
    static FontClass& getInstance();

    void unloadFont();
    Vector2 measure_size_char(char char_pressed);

    Font FONT_TYPE;
    float FONT_SIZE;
    float FONT_SPACING;

  private:
    FontClass() {
      FONT_TYPE = LoadFont("assets/font/courier_new.ttf");
      FONT_SIZE = 16;
      FONT_SPACING = 0.0f;
      SetTextureFilter(FONT_TYPE.texture, TEXTURE_FILTER_BILINEAR);
    }
    FontClass(const FontClass&) = delete;
    FontClass& operator=(const FontClass&) = delete;
};