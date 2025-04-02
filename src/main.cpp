#include "raylib.h"

#include "text/piece_table.h"
#include "font/font.h"
#include "cursor/cursor.h"
#include "utils/utils.h"

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
  Utils utils;
  PieceTable text_storage;
  std::string empty_string;
  text_storage.updateOriginalBuffer(empty_string);

  char pressed_char;
  int keys;
  bool cursor_moved = false;
  bool drag_and_drop_view = false;

  while (!WindowShouldClose()) {
    pressed_char = GetCharPressed();

    if (pressed_char) {
      text_storage.updatePieces(pressed_char, cursor.current_pos, cursor_moved);
      cursor.current_col++;
      cursor.current_pos++;
      text_storage.updateRowSize(1, cursor.current_row, cursor.current_col);
    }

    keys = GetKeyPressed();
    if (keys) {
      cursor_moved = false;

      // * move cursor
      if (keys == KEY_LEFT && cursor.current_col > 0) {
        cursor.current_col -= 1;
        cursor.current_pos -= 1;
        cursor_moved = true;
      } else if (keys == KEY_RIGHT && cursor.current_col < text_storage.getRowSize(cursor.current_row)) {
        cursor.current_col += 1;
        cursor.current_pos += 1;
        cursor_moved = true;
      } else if (keys == KEY_UP && cursor.current_row > 0) {
        // TODO:
        std::tie(cursor.current_pos, cursor.current_col) = text_storage.findNewCursorPos(-1, cursor.current_col, cursor.current_row);
        cursor.current_row -= 1;
        cursor_moved = true;
      } else if (keys == KEY_DOWN && cursor.current_row < text_storage.getTotalRows()-1) {
        // TODO: 
        std::tie(cursor.current_pos, cursor.current_col) = text_storage.findNewCursorPos(+1, cursor.current_col, cursor.current_row);
        cursor.current_row += 1;
        cursor_moved = true;
      }

      // * update text
      if (keys == KEY_BACKSPACE && cursor.current_col > 0) {
        cursor.current_col -= 1;
        cursor.current_pos -= 1;
        cursor_moved = true;
        text_storage.deleteChar(cursor.current_pos);
        text_storage.updateRowSize(-1, cursor.current_row, cursor.current_col);
      } else if (keys == KEY_ENTER) {
        text_storage.insertNewRow(cursor.current_pos);
        cursor.current_col = 0;
        cursor.current_row += 1;
        cursor.current_pos += 1;
        text_storage.updateRowSize(0, cursor.current_row, cursor.current_col);
      }
    }

    utils.handleInteractions(text_storage, cursor);

    BeginDrawing();
      ClearBackground(RAYWHITE);
      text_storage.renderPieces();
      cursor.render();
    EndDrawing();
  }

  FontClass::getInstance().unloadFont();
}

void initRaylibWindow() {
  constexpr int WINDOW_WIDTH = 700;
  constexpr int WINDOW_HEIGHT = 800;

  SetConfigFlags(FLAG_WINDOW_RESIZABLE);

  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Text Editor");
  SetTargetFPS(60);
}