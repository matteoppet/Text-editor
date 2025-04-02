#include "utils.h"

void Utils::handleInteractions(PieceTable& text_storage, Cursor& cursor) {
  // open file button interaction
  if (IsFileDropped()) {
    FilePathList droppedFiles = LoadDroppedFiles();
    
    if (IsFileExtension(droppedFiles.paths[0], ".txt")) {
      // TODO: alert that it will eliminate everything
      text_storage.readDroppedFile(droppedFiles.paths[0], cursor);
    }

    UnloadDroppedFiles(droppedFiles);
  }
}

