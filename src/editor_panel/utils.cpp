#include "utils.h"

Color BACKGROUND_TOOLBAR = {212, 212, 212};

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
    for (auto& button : buttons) {
      if (mouse_x > button.rect.x && mouse_x < button.rect.x+button.rect.width && mouse_y > button.rect.y && mouse_y < button.rect.y+button.rect.height) {
        if (button.name == "save") {
          if (path_current_file_saved.empty()) {

            path_current_file_saved = openFileDialog();
            if (path_current_file_saved.empty()) { // if the user clicked cancel
              return;
            } 
          }

          text_storage.saveFile(path_current_file_saved);
        }
        else if (button.name == "save_as") {
          path_current_file_saved = openFileDialog();

          if (path_current_file_saved.empty()) { // if the user clicked cancel
            return;
          } 
    
          text_storage.saveFile(path_current_file_saved);
        }
        else if (button.name == "undo") {
          text_storage.undo(cursor);
        }
        else if (button.name == "redo") {
          text_storage.redo(cursor);
        }
        else if (button.name == "copy") {
          text_storage.copy(cursor);
        }
        else if (button.name == "paste") {
          text_storage.paste(cursor);
        }
      }
    }
  }
}

void Utils::renderToolPanel() {
  DrawRectangle(0, 0, GetScreenWidth(), 27, RAYWHITE);
  DrawRectangle(0, 4, GetScreenWidth(), 21, WHITE);

  for (auto& button : buttons) {
    DrawTexture(button.texture, button.rect.x, button.rect.y, WHITE);
  }
}

void Utils::InitToolbar() {
  float start_x = 0;
  float start_y = 5;

  for (size_t i=0; i<functionalities.size(); i++) {
    std::string name_functionality = functionalities[i];
    std::string path_to_load = "assets/icons/" + name_functionality + ".png";

    Button new_functionality;
    new_functionality.name = name_functionality;
    new_functionality.texture = LoadTexture(path_to_load.c_str());
    new_functionality.rect = {start_x, start_y, 22.0f, 20.0f};

    buttons.push_back(new_functionality);

    start_x += new_functionality.rect.width;
  }
}

std::string Utils::openFileDialog() {
  const char *file = tinyfd_saveFileDialog("Select File", "", 0, NULL, NULL);

  if (file) {
    return file;
  } else {
    return "";
  }
}