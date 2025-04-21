#include "raylib.h"

#include "text_area/text_area.h"
#include "font/font.h"
#include "text_area/cursor.h"
#include "editor_panel/utils.h"

void runMainLoop();
void initRaylibWindow();
void handleKeyboard(Cursor& cursor, Utils& editor_panel, PieceTable& text_area);

int main() {
  initRaylibWindow();
  runMainLoop();
  CloseWindow();
  return 0;
}

void runMainLoop() {
  Cursor cursor;
  Utils editor_panel;
  PieceTable text_area;

  while (!WindowShouldClose()) {
    handleKeyboard(cursor, editor_panel, text_area);

    editor_panel.handleInteractions(text_area, cursor);
   
    BeginDrawing();
      ClearBackground(WHITE);
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

void handleKeyboard(Cursor& cursor, Utils& editor_panel, PieceTable& text_area) {
  char pressed_char = GetCharPressed();
  int keys = GetKeyPressed();

  if (pressed_char) {
    if (text_area.text_selected.selected) {
      text_area.deleteFromSelection();
      cursor.cursor_moved = true;
      text_area.unselectText(cursor);
      text_area.updateRowSize(0, cursor.current_row, cursor.current_col);
    }
    std::string temp_string(1, pressed_char);
    text_area.updatePieces(temp_string, cursor.current_pos, cursor.cursor_moved, cursor.current_col);
    cursor.current_col++;
    cursor.current_pos++;
    text_area.updateRowSize(0, cursor.current_row, cursor.current_col);
  }

  if (keys) {
    cursor.cursor_moved = false;

    if (keys == KEY_LEFT && cursor.current_col > 0) {
      cursor.current_col -= 1;
      cursor.current_pos -= 1;
      cursor.cursor_moved = true;

      if (IsKeyDown(KEY_LEFT_SHIFT)) text_area.selectText(cursor, "left");
      else {
        if (text_area.text_selected.selected) text_area.unselectText(cursor);
      }
    }
    else if (keys == KEY_RIGHT && cursor.current_col < text_area.getRowSize(cursor.current_row)) {
      cursor.current_col += 1;
      cursor.current_pos += 1;
      cursor.cursor_moved = true;

      if (IsKeyDown(KEY_LEFT_SHIFT)) text_area.selectText(cursor, "right");
      else { 
        if (text_area.text_selected.selected) text_area.unselectText(cursor);
      }
    }
    else if (keys == KEY_UP && cursor.current_row > 0) {
      std::tie(cursor.current_pos, cursor.current_col) = text_area.findNewCursorPos(-1, cursor.current_col, cursor.current_row);
      cursor.current_row -= 1;
      cursor.cursor_moved = true;

      if (IsKeyDown(KEY_LEFT_SHIFT)) text_area.selectText(cursor, "up");
      else {
        if (text_area.text_selected.selected) text_area.unselectText(cursor);
      }
    }
    else if (keys == KEY_DOWN && cursor.current_row < text_area.getTotalRows()-1) {
      std::tie(cursor.current_pos, cursor.current_col) = text_area.findNewCursorPos(+1, cursor.current_col, cursor.current_row);
      cursor.current_row += 1;
      cursor.cursor_moved = true;

      if (IsKeyDown(KEY_LEFT_SHIFT)) text_area.selectText(cursor, "down");
      else {
        if (text_area.text_selected.selected) text_area.unselectText(cursor);
      }
    }
    else if (keys == KEY_BACKSPACE) {
      if (text_area.text_selected.selected) {
        text_area.deleteFromSelection();
        cursor.cursor_moved = true;
        text_area.unselectText(cursor);
        text_area.updateRowSize(0, cursor.current_row, cursor.current_col);
      } else {
        if (cursor.current_col == 0 && cursor.current_row == 0) {
          
        } else {
          if (cursor.current_col == 0) {
            cursor.current_pos -= 1;
            cursor.current_row -= 1;
            cursor.current_col = text_area.getRowSize(cursor.current_row);
          } else {
            cursor.current_col -= 1;
            cursor.current_pos -= 1;
          }
          cursor.cursor_moved = true;
          text_area.deleteChar(cursor.current_pos, cursor.current_col);
          text_area.updateRowSize(-1, cursor.current_row, cursor.current_col);
        }
        }
    }
    else if (keys == KEY_ENTER) {
      text_area.insertNewRow(cursor.current_pos);
      cursor.current_col = 0;
      cursor.current_row += 1;
      cursor.current_pos += 1;
      text_area.updateRowSize(0, cursor.current_row, cursor.current_col);
    }
    else if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_C)) {
      text_area.copy(cursor);
    }
    else if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_V)) {
      text_area.paste(cursor);
    }
    else if (IsKeyDown(KEY_TAB)) {
      text_area.updatePieces("    ", cursor.current_pos, cursor.cursor_moved, cursor.current_col);
      cursor.current_pos += 4;
      cursor.current_col += 4;
      text_area.updateRowSize(0, cursor.current_row, cursor.current_col);
    }
  }
}