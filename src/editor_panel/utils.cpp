#include "utils.h"

void Utils::handleInteractions(PieceTable& text_storage, Cursor& cursor) {
  size_t mouse_x = GetMousePosition().x;
  size_t mouse_y = GetMousePosition().y;

  if (IsFileDropped()) {
    FilePathList droppedFiles = LoadDroppedFiles();
    
    if (IsFileExtension(droppedFiles.paths[0], ".txt")) {
      text_storage.readDroppedFile(droppedFiles.paths[0], cursor);
    }

    UnloadDroppedFiles(droppedFiles);
  }

  if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
    // save button
    if (mouse_x > save_file_button.x && mouse_x < save_file_button.x+save_file_button.width && mouse_y > save_file_button.y && mouse_y < save_file_button.y+save_file_button.height) {      
      if (path_current_file_saved.empty()) {
        path_current_file_saved = openFileDialog();
        if (path_current_file_saved.empty()) { // if the user clicked cancel
          return;
        } 
      }
      text_storage.saveFile(path_current_file_saved);
    }
    // save as button
    else if (mouse_x > save_as_file_button.x && mouse_x < save_as_file_button.x+save_as_file_button.width && mouse_y > save_as_file_button.y && mouse_y < save_as_file_button.y+save_as_file_button.height) {      
      path_current_file_saved = openFileDialog();
      
      if (path_current_file_saved.empty()) { // if the user clicked cancel
        return;
      } 

      text_storage.saveFile(path_current_file_saved);
    }
    // undo button
    else if (mouse_x > undo_button.x && mouse_x < undo_button.x+undo_button.width && mouse_y > undo_button.y && mouse_y < undo_button.y+undo_button.height) {
      text_storage.undo(cursor);
    }
    // redo button
    else if (mouse_x > redo_button.x && mouse_x < redo_button.x+redo_button.width && mouse_y > redo_button.y && mouse_y < redo_button.y+redo_button.height) {
      text_storage.redo(cursor);
    }
  }
}

void Utils::renderToolPanel() {
  // BACKGROUND tool panel
  DrawRectangle(0, 0, GetScreenWidth(), 20, GRAY);

  DrawRectangle(save_file_button.x, save_file_button.y, save_file_button.width, save_file_button.height, GRAY);
  DrawTextEx(font_text_tool_panel, "Save", {save_file_button.x+3, save_file_button.y+3}, 14, 0, BLACK);
  
  DrawRectangle(save_as_file_button.x, save_as_file_button.y, save_as_file_button.width, save_as_file_button.height, GRAY);
  DrawTextEx(font_text_tool_panel, "Save as", {save_as_file_button.x+3, save_as_file_button.y+3}, 14, 0, BLACK);

  DrawRectangle(undo_button.x, undo_button.y, undo_button.width, undo_button.height, GRAY);
  DrawTextEx(font_text_tool_panel, "Undo", {undo_button.x+3, undo_button.y+3}, 14, 0, BLACK);

  DrawRectangle(redo_button.x, redo_button.y, redo_button.width, redo_button.height, GRAY);
  DrawTextEx(font_text_tool_panel, "Redo", {redo_button.x+3, redo_button.y+3}, 14, 0, BLACK);

}

std::string Utils::openFileDialog() {
  const char *file = tinyfd_openFileDialog("Open File", "", 0, NULL, NULL, 0);

  if (file) {
    return file;
  } else {
    return "";
  }
}