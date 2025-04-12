#include "raylib.h"

#include "text_area/text_area.h"
#include "font/font.h"
#include "text_area/cursor.h"
#include "editor_panel/utils.h"

void runMainLoop();
void initRaylibWindow();

int main() {
  initRaylibWindow();
  runMainLoop();
  CloseWindow();
  return 0;
}

void runMainLoop() {
  // Init classes
  Cursor cursor;
  Utils editor_panel;
  PieceTable text_area;
  std::string empty_string;
  text_area.updateOriginalBuffer(empty_string);

  char pressed_char;
  int keys;

  while (!WindowShouldClose()) {
    pressed_char = GetCharPressed();

    if (pressed_char) {
      if (text_area.text_selected.selected) text_area.unselectText(cursor);
      text_area.updatePieces(pressed_char, cursor.current_pos, cursor.cursor_moved, cursor.current_col);
      cursor.current_col++;
      cursor.current_pos++;
      text_area.updateRowSize(1, cursor.current_row, cursor.current_col);
    }

    keys = GetKeyPressed();
    if (keys) {
      cursor.cursor_moved = false;
      
      // * select text
      if (IsKeyDown(KEY_LEFT_SHIFT) && IsKeyDown(KEY_LEFT) && cursor.current_col > 0) {
        cursor.current_col -= 1;
        text_area.selectText(cursor, cursor.current_row, cursor.current_col+1);
      } else if (IsKeyDown(KEY_LEFT_SHIFT) && IsKeyDown(KEY_RIGHT) && cursor.current_col < text_area.getRowSize(cursor.current_row)) {
        cursor.current_col += 1;
        text_area.selectText(cursor, cursor.current_row, cursor.current_col-1);
      } else if (IsKeyDown(KEY_LEFT_SHIFT) && IsKeyDown(KEY_UP) && cursor.current_row > 0) {
        cursor.current_row -= 1;
        text_area.selectText(cursor, cursor.current_row+1, cursor.current_col);
      } else if (IsKeyDown(KEY_LEFT_SHIFT) && IsKeyDown(KEY_DOWN) && cursor.current_row < text_area.getTotalRows()) {
        cursor.current_row += 1;
        text_area.selectText(cursor, cursor.current_row-1, cursor.current_col);
      }
      // * move cursorr
      else if (keys == KEY_LEFT && cursor.current_col > 0) {
        cursor.current_col -= 1;
        cursor.current_pos -= 1;
        cursor.cursor_moved = true;
      } else if (keys == KEY_RIGHT && cursor.current_col < text_area.getRowSize(cursor.current_row)) {
        cursor.current_col += 1;
        cursor.current_pos += 1;
        cursor.cursor_moved = true;
      } else if (keys == KEY_UP && cursor.current_row > 0) {
        std::tie(cursor.current_pos, cursor.current_col) = text_area.findNewCursorPos(-1, cursor.current_col, cursor.current_row);
        cursor.current_row -= 1;
        cursor.cursor_moved = true;
      } else if (keys == KEY_DOWN && cursor.current_row < text_area.getTotalRows()-1) {
        std::tie(cursor.current_pos, cursor.current_col) = text_area.findNewCursorPos(+1, cursor.current_col, cursor.current_row);
        cursor.current_row += 1;
        cursor.cursor_moved = true;
      }

      // * update text
      if (keys == KEY_BACKSPACE && cursor.current_col > 0) {
        cursor.current_col -= 1;
        cursor.current_pos -= 1;
        cursor.cursor_moved = true;
        text_area.deleteChar(cursor.current_pos, cursor.current_col);
        text_area.updateRowSize(-1, cursor.current_row, cursor.current_col);
      } else if (keys == KEY_ENTER) {
        text_area.insertNewRow(cursor.current_pos);
        cursor.current_col = 0;
        cursor.current_row += 1;
        cursor.current_pos += 1;
        text_area.updateRowSize(0, cursor.current_row, cursor.current_col);
      }
    }

    editor_panel.handleInteractions(text_area, cursor);
   
    BeginDrawing();
      ClearBackground(RAYWHITE);

      text_area.render(cursor);
      editor_panel.renderToolPanel();
    EndDrawing();
  }

  FontClass::getInstance().unloadFont();
  text_area.freeMemory();
}

void initRaylibWindow() {
  constexpr int WINDOW_WIDTH = 700;
  constexpr int WINDOW_HEIGHT = 800;

  SetConfigFlags(FLAG_WINDOW_RESIZABLE);

  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Text Editor");
  SetTargetFPS(60);
}