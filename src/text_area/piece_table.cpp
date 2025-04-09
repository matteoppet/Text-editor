#include "piece_table.h"

void PieceTable::updateOriginalBuffer(const std::string& text) {
  if (!text.empty()) {
    Piece* new_piece = new Piece({false, &originalBuffer, 0, originalBuffer.size()});
    pieces.push_back(new_piece);
  } else {
    Piece* new_piece = new Piece({false, &originalBuffer, 0, 0});
    pieces.push_back(new_piece);
  }
}

void PieceTable::updatePieces(const char char_to_insert, size_t position_to_insert, bool insert_new_piece, size_t cursor_col) {
  addBuffer += char_to_insert;
  redo_stack.clear();

  size_t offset = 0;
  std::vector<Piece*> new_pieces;

  for (auto piece : pieces) {
    // * case 1: split pieces
    if (offset < position_to_insert && offset+piece->length > position_to_insert) {
      size_t local_offset = position_to_insert-offset;
      
      Piece* first_part_piece = new Piece({piece->fromAddBuffer, piece->buffer, piece->start, local_offset});
      new_pieces.push_back(first_part_piece);

      Piece* new_part_piece = new Piece({true, &addBuffer, addBuffer.length()-1, 1});
      new_pieces.push_back(new_part_piece);

      ActionRecordUndo* new_action = new ActionRecordUndo{ActionType::SPLIT, new_pieces.size()-1, cursor_col, new_part_piece, piece, nullptr};
      undo_stack.push_back(new_action);

      Piece* last_part_piece = new Piece({piece->fromAddBuffer, piece->buffer, piece->start+local_offset, piece->length-local_offset});
      new_pieces.push_back(last_part_piece);

    }
    // * case 2: increase length one piece
    else if (offset+piece->length == position_to_insert) {
      std::string substring = piece->buffer->substr(piece->start, piece->length);

      // insert new piece if the cursor was moved or the piece to increase is not from the addBuffer
      if (insert_new_piece || !piece->fromAddBuffer || substring.compare("\n") == 0) {
        new_pieces.push_back(piece);
        Piece* new_piece = new Piece({true, &addBuffer, addBuffer.length()-1, 1});
        new_pieces.push_back(new_piece);
        
        ActionRecordUndo* new_action = new ActionRecordUndo{ActionType::INSERT, new_pieces.size()-1, cursor_col, new_piece, nullptr, nullptr};
        undo_stack.push_back(new_action);
      }
      else {
        piece->length += 1;
        new_pieces.push_back(piece);
      }
    }
    else {
      new_pieces.push_back(piece);
    }
    offset += piece->length;
  }

  pieces = std::move(new_pieces);
}

void PieceTable::deleteChar(size_t position_to_delete, size_t cursor_col) { // TODO: don't do the loop each time a backspace key is pressed to decrease length piece
   size_t cursor_pos_before_deletion = position_to_delete+1;
  size_t offset = 0;
  std::vector<Piece*> new_pieces;
  bool removed = false;

  for (auto piece : pieces) {
    if (offset < cursor_pos_before_deletion+1 && offset+piece->length > cursor_pos_before_deletion && !removed) {
      size_t local_offset = position_to_delete-offset;
      
      Piece* first_part_piece = new Piece({true, &addBuffer, piece->start, local_offset});
      new_pieces.push_back(first_part_piece);

      Piece* second_part_piece = new Piece({true, &addBuffer, piece->start+local_offset+1, piece->length-local_offset-1});
      new_pieces.push_back(second_part_piece);

      ActionRecordUndo* new_action = new ActionRecordUndo{ActionType::DELETE, new_pieces.size()-1, cursor_col, nullptr, piece, nullptr};
      undo_stack.push_back(new_action);

      removed = true;

    } else if (offset+piece->length == cursor_pos_before_deletion && !removed) {   
      if (piece->length > 1) {
        ActionRecordUndo* new_action = new ActionRecordUndo(ActionType::DELETE, new_pieces.size()-1, cursor_col, nullptr, nullptr, piece);
        undo_stack.push_back(new_action);

        piece->length -= 1;
        new_pieces.push_back(piece);
        removed = true;
      } else {
        delete piece;
        removed = true;
      }
    } else {
      new_pieces.push_back(piece);
    }
    offset += piece->length;
  }

  pieces = std::move(new_pieces);
}

void PieceTable::render(Cursor& cursor) {
  float start_x = 5;
  float start_y = 25;
  float length_current_word = 0;
  FontClass& instance_font_class = FontClass::getInstance();

  cursor.render(start_x, start_y);
  
  // draw pieces
  for (const auto& piece : pieces) {
    std::string substring = piece->buffer->substr(piece->start, piece->length);

    if (substring.compare("\n") == 0) {
      start_y += 18;
      start_x = 5;
    } else {
      length_current_word = instance_font_class.measure_size_char(substring[0]).x*substring.length();
      DrawTextEx(instance_font_class.FONT_TYPE, substring.c_str(), {start_x, start_y}, instance_font_class.FONT_SIZE, instance_font_class.FONT_SPACING, BLACK);
      start_x += length_current_word;
    }
  }
}

void PieceTable::insertNewRow(size_t cursor_pos) {
  addBuffer += "\n";
  size_t offset = 0;
  bool inserted = false;
  std::vector<Piece*> new_pieces;

  for (auto piece : pieces) {
    if (offset < cursor_pos && offset+piece->length > cursor_pos && !inserted) {
      size_t local_offset = cursor_pos-offset;
      
      Piece* first_part_piece = new Piece({piece->fromAddBuffer, &addBuffer, piece->start, local_offset});
      new_pieces.push_back(first_part_piece);

      Piece* new_part_piece = new Piece({true, &addBuffer, addBuffer.length()-1, 1});
      new_pieces.push_back(new_part_piece);

      Piece* last_part_piece = new Piece({piece->fromAddBuffer, &addBuffer, piece->start+local_offset, piece->length-local_offset});
      new_pieces.push_back(last_part_piece);

      inserted = true;
    } else if (cursor_pos-1 == offset && !inserted) {
      Piece * new_piece = new Piece({true, &addBuffer, addBuffer.length()-1, 1});
      new_pieces.push_back(new_piece),
      new_pieces.push_back(piece);

      inserted = true;
    }
    else if (cursor_pos == offset+piece->length && !inserted) {

      new_pieces.push_back(piece);
      Piece * new_piece = new Piece({true, &addBuffer, addBuffer.length()-1, 1});
      new_pieces.push_back(new_piece);
    }
    else {
      new_pieces.push_back(piece);
    }
    offset += piece->length;
  }

  pieces= std::move(new_pieces);
}
 
void PieceTable::updateRowSize(int action, int cursor_row, int cursor_col) {
  if (action == 0) {
    size_t count_rows = 0;
    std::map<int,int> new_size_rows;
    new_size_rows[0] = 0;

    for (auto piece : pieces) {
      std::string substring = piece->buffer->substr(piece->start, piece->length);
    
      if (substring.compare("\n") == 0) {
        count_rows++;
      } else {
        new_size_rows[count_rows] += substring.length();
      }
    }

    size_rows = new_size_rows;
  } else {
    size_rows[cursor_row] += action;
  }
}

int PieceTable::getRowSize(int row) {
  return size_rows[row];
}

int PieceTable::getTotalRows() {
  auto last_element = size_rows.end();
  return last_element->first;
}

std::pair<int, int> PieceTable::findNewCursorPos(int action, int cursor_col, int cursor_row) {
  size_t row_to_search = cursor_row+action;
  size_t rows_before = 0;

  for (auto it : size_rows) {
    if (it.first < row_to_search) {
      rows_before += it.second;
    }
  }

  if (size_rows[row_to_search] >= cursor_col) { // row_to_search longer or equal than cursor_col
    int new_cursor_pos = rows_before+cursor_col+row_to_search;
    
    return {new_cursor_pos, cursor_col};
  } else {  // row_to_search shorter than cursor_col
    int new_cursor_pos = rows_before+size_rows[row_to_search];
    int new_cursor_col = size_rows[row_to_search];
    if (row_to_search > 0) new_cursor_pos += row_to_search;

    return {new_cursor_pos, new_cursor_col};
  }

  return {0,0};
}

void PieceTable::readDroppedFile(std::string file_path, Cursor& cursor) {
  reset();
  cursor.reset();

  // open file for reading
  std::string new_text;
  std::ifstream FileToRead(file_path);

  // append the text that is inside the file into the originalBuffer
  while (getline(FileToRead, new_text)) {
    originalBuffer += new_text;
    originalBuffer += "\n";
  }

  // create pieces from the originalBuffer string
  size_t offset_start = 0;
  size_t count_length_piece = 0;
  size_t lines = 0;
  for (int i=0; i<originalBuffer.length(); i++) {
    if (originalBuffer[i] == '\n') {
      count_length_piece++;

      size_rows[lines] = count_length_piece-1;
      lines++;

      Piece* text_piece = new Piece({false, &originalBuffer, offset_start, count_length_piece-1}); 
      pieces.push_back(text_piece);

      offset_start += count_length_piece;
      count_length_piece = 0;
      
      Piece* new_line_piece = new Piece({false, &originalBuffer, offset_start-1, 1});
      pieces.push_back(new_line_piece);

    } else {
      count_length_piece++;
    }
  }

} 

void PieceTable::reset() {
  originalBuffer.clear();
  addBuffer.clear();
  pieces.clear();
  size_rows.clear();
}

void PieceTable::saveFile(std::string file_path) {
  // opening file to save all the text
  std::ofstream outFile(file_path);
  for (const auto& piece : pieces) {
    if (piece->fromAddBuffer) {
      outFile << addBuffer.substr(piece->start, piece->length);
    } else {
      outFile << originalBuffer.substr(piece->start, piece->length);
    }
  }

  outFile.close();

  std::cout << "file saved successfully" << std::endl;
}

void PieceTable::undo(Cursor& cursor) {
  auto piece_to_undo = undo_stack.back();
  size_t offset_piece = piece_to_undo->index_in_piece;

  if (piece_to_undo->type == ActionType::SPLIT) {
    ActionRecordRedo* new_action = new ActionRecordRedo{ActionType::SPLIT, offset_piece-1, cursor.current_col, {pieces[offset_piece-1], pieces[offset_piece], pieces[offset_piece+1]}, {}, nullptr};
    redo_stack.push_back(new_action);

    pieces.erase(pieces.begin()+offset_piece+1);
    pieces.erase(pieces.begin()+offset_piece+1);
    pieces.insert(pieces.begin()+offset_piece, piece_to_undo->piece_before_splitting);
    pieces.erase(pieces.begin()+offset_piece-1);
    
    updateRowSize(0,0,0);
    std::tie(cursor.current_pos, cursor.current_col) = findNewCursorPos(0, piece_to_undo->cursor_col, cursor.current_row);
    undo_stack.pop_back();
  }
  else if (piece_to_undo->type == ActionType::DELETE) {
    if (piece_to_undo->piece_before_splitting == nullptr) { // decreased length piece
      piece_to_undo->piece_before_deletion->length += 1;
      cursor.current_col += 1;
      cursor.current_pos += 1;
      cursor.cursor_moved = true;
      undo_stack.pop_back();
      
      ActionRecordRedo* new_action = new ActionRecordRedo(ActionType::DELETE, offset_piece, cursor.current_col, {}, {}, piece_to_undo->piece_before_deletion);
      redo_stack.push_back(new_action);

    } else {
      ActionRecordRedo* new_action = new ActionRecordRedo(ActionType::DELETE, offset_piece, cursor.current_col, {}, {pieces[offset_piece-1], pieces[offset_piece]}, nullptr);
      redo_stack.push_back(new_action);
      
      pieces.erase(pieces.begin()+offset_piece+1);
      pieces.insert(pieces.begin()+offset_piece, piece_to_undo->piece_before_splitting);
      pieces.erase(pieces.begin()+offset_piece-1);
      
      updateRowSize(0,0,0);
      std::tie(cursor.current_pos, cursor.current_col) = findNewCursorPos(0, piece_to_undo->cursor_col, cursor.current_row);
      undo_stack.pop_back();
    }
  }
  else if (piece_to_undo->type == ActionType::INSERT) {
    ActionRecordRedo* new_action = new ActionRecordRedo(ActionType::INSERT, offset_piece, cursor.current_col, {}, {}, nullptr);
    new_action->piece_to_insert = pieces[offset_piece];
    redo_stack.push_back(new_action);

    pieces.erase(pieces.begin()+offset_piece);

    updateRowSize(0,0,0);
    std::tie(cursor.current_pos, cursor.current_col) = findNewCursorPos(0, piece_to_undo->cursor_col, cursor.current_row);
    undo_stack.pop_back();
  }
}

void PieceTable::redo(Cursor& cursor) {
  auto piece_to_redo = redo_stack.back();
  size_t offset_piece = piece_to_redo->index_in_piece;

  if (piece_to_redo->type == ActionType::SPLIT) {
    pieces.erase(pieces.begin()+offset_piece);

    for (size_t i=0; i<piece_to_redo->piece_splitted.size(); i++) {
      pieces.insert(pieces.begin()+(offset_piece+i), piece_to_redo->piece_splitted[i]);
    }

    updateRowSize(0,0,0);
    std::tie(cursor.current_pos, cursor.current_col) = findNewCursorPos(0, piece_to_redo->cursor_col, cursor.current_row);
    redo_stack.pop_back();
  }
  else if (piece_to_redo->type == ActionType::DELETE) {
    if (piece_to_redo->piece_deleted.empty()) {
      piece_to_redo->piece_decreased_length->length -= 1;
      cursor.current_col += 1;
      cursor.current_pos += 1;
      cursor.cursor_moved = true;
      redo_stack.pop_back();
    } else {
      pieces.erase(pieces.begin()+offset_piece);
      for (size_t i=0; i<piece_to_redo->piece_deleted.size(); i++) {
        pieces.insert(pieces.begin()+((offset_piece-1)+i), piece_to_redo->piece_deleted[i]);
      }
      
      cursor.cursor_moved = true;
      updateRowSize(0,0,0);
      redo_stack.pop_back();
    }
  }
  else if (piece_to_redo->type == ActionType::INSERT) {
    pieces.insert(pieces.begin()+offset_piece, piece_to_redo->piece_to_insert);

    updateRowSize(0,0,0);
    std::tie(cursor.current_pos, cursor.current_col) = findNewCursorPos(0, piece_to_redo->cursor_col, cursor.current_row);
    cursor.cursor_moved = true;
    redo_stack.pop_back();
  }
}

void PieceTable::freeMemory() {
  // free pieces
  for (auto& piece : pieces) {
    delete piece;
  }
  
  // free undo_stack
  for (auto& action_undo : undo_stack) {
    delete action_undo;
  }
}
