#include "text_area.h"

void PieceTable::updateOriginalBuffer(const std::string& text) {
  if (!text.empty()) {
    Piece* new_piece = new Piece({false, &originalBuffer, 0, originalBuffer.size()});
    pieces.push_back(new_piece);
  } else {
    Piece* new_piece = new Piece({false, &originalBuffer, 0, 0});
    pieces.push_back(new_piece);
  }
}

void PieceTable::updatePieces(std::string text_to_insert, size_t position_to_insert, bool insert_new_piece, size_t cursor_col) {
  size_t length_text = text_to_insert.length();
  addBuffer.append(text_to_insert);
  redo_stack.clear();

  if (pieces.size() == 0) {
    Piece* new_piece = new Piece({true, &addBuffer, addBuffer.length()-length_text, length_text});
    pieces.push_back(new_piece);
  } 
  else {
    size_t offset = 0;
    std::vector<Piece*> new_pieces;
    new_pieces.reserve(pieces.size());

    for (auto& piece : pieces) {
      // case 1: split pieces
      if (offset < position_to_insert && offset+piece->length > position_to_insert) {
        size_t local_offset = position_to_insert-offset;

        Piece* first_part_piece = new Piece({piece->fromAddBuffer, piece->buffer, piece->start, local_offset});
        new_pieces.push_back(first_part_piece);

        Piece* new_part_piece = new Piece({true, &addBuffer, addBuffer.length()-length_text, length_text});
        new_pieces.push_back(new_part_piece);

        Piece* last_part_piece = new Piece({piece->fromAddBuffer, piece->buffer, piece->start+local_offset, piece->length-local_offset});
        new_pieces.push_back(last_part_piece);

        // registry action into undo stack
        ActionRecordUndo* new_action = new ActionRecordUndo{ActionType::SPLIT, new_pieces.size(), cursor_col, new_part_piece, piece, nullptr};
        undo_stack.push_back(new_action);
      }
      // case 2: increase length of the piece
      else if (offset+piece->length == position_to_insert) {
        std::string substring = piece->buffer->substr(piece->start, piece->length);

        // insert new piece in it meets some rules
        if (insert_new_piece || !piece->fromAddBuffer || substring.compare("\n") == 0) {
          new_pieces.push_back(piece);
          Piece* new_piece = new Piece({true, &addBuffer, addBuffer.length()-length_text, length_text});
          new_pieces.push_back(new_piece);

            // registry action into undo stack
          ActionRecordUndo* new_action = new ActionRecordUndo{ActionType::INSERT, new_pieces.size()-1, cursor_col, new_piece, nullptr, nullptr};
          undo_stack.push_back(new_action);
        }
        else {
          piece->length += 1;
          new_pieces.push_back(piece);
        }
      }
      // case 3: just add the piece as same as before
      else {
        new_pieces.push_back(piece);
      }
      offset += piece->length;
    }

    pieces.clear();
    pieces = std::move(new_pieces);
  }
}

void PieceTable::deleteChar(size_t position_to_delete, size_t cursor_col) {
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

void PieceTable::deleteFromSelection() { // TODO: add this deletion to the undo stack
  int anchor_point_pos_in_buffer = findNewCursorPos(0, text_selected.anchor_point.x, text_selected.anchor_point.y).first;
  int active_point_pos_in_buffer = findNewCursorPos(0, text_selected.active_point.x, text_selected.active_point.y).first;

  size_t selection_start = std::min(anchor_point_pos_in_buffer, active_point_pos_in_buffer);
  size_t selection_end = std::max(anchor_point_pos_in_buffer, active_point_pos_in_buffer);

  std::vector<Piece*> new_pieces;
  new_pieces.reserve(pieces.size());
  size_t offset = 0;
  for (const auto* piece : pieces) {
    size_t piece_end_offset = offset + piece->length;

    // entirely before the selection
    if (piece_end_offset <= selection_start) {
      new_pieces.push_back(new Piece(*piece));
    }
    // entirely after the selection
    else if (offset >= selection_end) {
      new_pieces.push_back(new Piece(*piece));
    }
    else {
      // piece needs splitting
      if (offset < selection_start && piece_end_offset > selection_end) {
        size_t length_first_part = selection_start - offset;
        if (length_first_part > 0) new_pieces.push_back(new Piece({piece->fromAddBuffer, piece->buffer, piece->start, length_first_part}));

        size_t length_second_part = piece_end_offset - selection_end;
        if (length_second_part > 0) {;
          size_t start_second_part = (piece->start+length_first_part) + (selection_end - selection_start);
          new_pieces.push_back(new Piece({piece->fromAddBuffer, piece->buffer, start_second_part, length_second_part}));
        }
      }
      else if (offset < selection_start) {
        size_t new_length = selection_start - offset;
        if (new_length > 0) new_pieces.push_back(new Piece({piece->fromAddBuffer, piece->buffer, piece->start, new_length}));
      }
      else if (piece_end_offset > selection_end) {
        size_t skipped_length = selection_end - offset;
        size_t new_length = piece->length - skipped_length;
        if (new_length > 0) {
          size_t new_start_buffer = piece->start + skipped_length;
          new_pieces.push_back(new Piece({piece->fromAddBuffer, piece->buffer, new_start_buffer, new_length}));
        }
      }
    }
    offset += piece->length;
  }

  for (auto* p : pieces) {
    delete p; 
  }
  pieces.clear();
  
  pieces = std::move(new_pieces);
}

void PieceTable::render(Cursor& cursor) {
  float start_x = 30;
  float start_y = 30;
  float length_current_word = 0;
  FontClass& instance_font_class = FontClass::getInstance();
  size_t count = 0;

  // mouse scroll text
  float mouse_wheel_movements = GetMouseWheelMove(); // -1 down, 1 up
  if (mouse_wheel_movements != 0) {
    mouse_scroll_new_position -=  (int)(GetMouseWheelMove()*mouse_scroll_speed);

    if (mouse_scroll_new_position < 0) mouse_scroll_new_position = 0;
  }
  start_y -= mouse_scroll_new_position;

  // render background text selected
  if (text_selected.selected) {
    /* 
      active point col < anchor point = rectangle from active point to anchor point
      active point col > anchor point = rectangle from anchor point to active point

      active point row < anchor point row = rectangle from active point to the end of that row to from the start of the anchor point row to the point
      active point row > anchor point row = rectangle from anchor point to the end of that row to from the start of the anchor point row to the point
    */

    float pos_x;
    float pos_y;
    float width;
    float height;

    // same row
    if (text_selected.active_point.y == text_selected.anchor_point.y) {
      pos_y = start_y+(text_selected.active_point.y*instance_font_class.measure_size_char('a').y);
      height = instance_font_class.measure_size_char('a').y;

      if (text_selected.active_point.x < text_selected.anchor_point.x) {
        pos_x = start_x+(text_selected.active_point.x*instance_font_class.measure_size_char('a').x);
        width = instance_font_class.measure_size_char('a').x*(text_selected.anchor_point.x-text_selected.active_point.x);
      } else if (text_selected.active_point.x > text_selected.anchor_point.x) {
        pos_x = start_x+(text_selected.anchor_point.x*instance_font_class.measure_size_char('a').x);
        width = instance_font_class.measure_size_char('a').x*(text_selected.active_point.x-text_selected.anchor_point.x);
      }

      DrawRectangle(pos_x, pos_y, width, height, YELLOW);
    }
    // different rows
    else {
      // row up
      if (text_selected.active_point.y < text_selected.anchor_point.y) {

        for (int i=text_selected.active_point.y; i<=text_selected.anchor_point.y; i++) {
          pos_y = start_y+(i*instance_font_class.measure_size_char('a').y);
          height = instance_font_class.measure_size_char('a').y;

          if (i == text_selected.active_point.y) {
            pos_x = start_x+(text_selected.active_point.x*instance_font_class.measure_size_char('a').x);
            width = instance_font_class.measure_size_char('a').x*(getRowSize(text_selected.active_point.y)-text_selected.active_point.x);
          } else if (i == text_selected.anchor_point.y) {
            pos_x = start_x;
            width = instance_font_class.measure_size_char('a').x*text_selected.anchor_point.x;
          } else {
            pos_x = start_x;
            width = instance_font_class.measure_size_char('a').x*getRowSize(i);
          }
          
          Rectangle current_rectangle = {pos_x, pos_y, width, height};
          DrawRectangleRec(current_rectangle, YELLOW);
        }
      }
      // row down
      else if (text_selected.active_point.y > text_selected.anchor_point.y) {

        for (int i=text_selected.anchor_point.y; i<=text_selected.active_point.y; i++) {
          pos_y = start_y+(i*instance_font_class.measure_size_char('a').y);
          height = instance_font_class.measure_size_char('a').y;
          
          if (i == text_selected.anchor_point.y) {
            pos_x = start_x+(text_selected.anchor_point.x*instance_font_class.measure_size_char('a').x);
            width = instance_font_class.measure_size_char('a').x*(getRowSize(text_selected.anchor_point.y)-text_selected.anchor_point.x);
          } else if (i == text_selected.active_point.y) {
            pos_x = start_x;
            width = instance_font_class.measure_size_char('a').x*text_selected.active_point.x;
          } else {
            pos_x = start_x;
            width = instance_font_class.measure_size_char('a').x*getRowSize(i);
          }
          
          Rectangle current_rectangle = {pos_x, pos_y, width, height};
          DrawRectangleRec(current_rectangle, YELLOW);
        }
      }
    }
  }

  cursor.render(start_x, start_y);

  // draw background count lines
  DrawRectangle(0, 30, 20, (float)GetScreenHeight(), RAYWHITE),

  // draw pieces
  DrawTextEx(instance_font_class.FONT_TYPE, TextFormat("%d", count), Vector2{5, start_y}, instance_font_class.FONT_SIZE, instance_font_class.FONT_SPACING, BLACK);
  for (const auto& piece : pieces) {
    std::string substring = piece->buffer->substr(piece->start, piece->length);

    if (substring.compare("\n") == 0) {
      start_y += 18;

      count++;
      DrawTextEx(instance_font_class.FONT_TYPE, TextFormat("%d", count), Vector2{5, start_y}, instance_font_class.FONT_SIZE, instance_font_class.FONT_SPACING, BLACK);

      start_x = 30; 
    } else {
      length_current_word = instance_font_class.measure_size_char(substring[0]).x*substring.length();
      DrawTextEx(instance_font_class.FONT_TYPE, substring.c_str(), {start_x, start_y}, instance_font_class.FONT_SIZE, instance_font_class.FONT_SPACING, BLACK);
      start_x += length_current_word;
    }
  }
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
}

void PieceTable::undo(Cursor& cursor) {
  if (!undo_stack.empty()) {
    auto piece_to_undo = undo_stack.back();
    size_t offset_piece = piece_to_undo->index_in_piece;

    if (piece_to_undo->type == ActionType::SPLIT) {
      ActionRecordRedo* new_action = new ActionRecordRedo{ActionType::SPLIT, offset_piece-1, cursor.current_col, {pieces[offset_piece-1], pieces[offset_piece], pieces[offset_piece+1]}, {}, nullptr, piece_to_undo};
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
        
        ActionRecordRedo* new_action = new ActionRecordRedo(ActionType::DELETE, offset_piece, cursor.current_col, {}, {}, piece_to_undo->piece_before_deletion, piece_to_undo);
        redo_stack.push_back(new_action);

      } else {
        ActionRecordRedo* new_action = new ActionRecordRedo(ActionType::DELETE, offset_piece, cursor.current_col, {}, {pieces[offset_piece-1], pieces[offset_piece]}, nullptr, piece_to_undo);
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
      ActionRecordRedo* new_action = new ActionRecordRedo(ActionType::INSERT, offset_piece, cursor.current_col, {}, {}, nullptr, piece_to_undo);
      new_action->piece_to_insert = pieces[offset_piece];
      redo_stack.push_back(new_action);

      pieces.erase(pieces.begin()+offset_piece);

      updateRowSize(0,0,0);
      std::tie(cursor.current_pos, cursor.current_col) = findNewCursorPos(0, piece_to_undo->cursor_col, cursor.current_row);
      undo_stack.pop_back();
    }
  }
}

void PieceTable::redo(Cursor& cursor) { // TODO: add the piece to the UNDO stack 
  if (!redo_stack.empty()) {
    auto piece_to_redo = redo_stack.back();
    size_t offset_piece = piece_to_redo->index_in_piece;
    undo_stack.push_back(piece_to_redo->undo_action);

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
}

void PieceTable::selectText(Cursor& cursor, std::string direction) {
  if (!text_selected.selected) { // setting up anchor point and active point
    text_selected.selected = true;

    if (direction == "left") text_selected.anchor_point = {(float)cursor.current_col+1, (float)cursor.current_row};
    else if (direction == "right") text_selected.anchor_point = {(float)cursor.current_col-1, (float)cursor.current_row};
    else if (direction == "up") text_selected.anchor_point = {(float)cursor.current_col, (float)cursor.current_row+1};
    else if (direction == "down") text_selected.anchor_point = {(float)cursor.current_col, (float)cursor.current_row-1};

    text_selected.active_point = {(float)cursor.current_col, (float)cursor.current_row};
  } else { // move active point
    text_selected.active_point = {(float)cursor.current_col, (float)cursor.current_row};
  }
}

void PieceTable::unselectText(Cursor& cursor) { 
  cursor.current_col = text_selected.active_point.x;
  cursor.current_row = text_selected.active_point.y;
  cursor.current_pos = findNewCursorPos(0, cursor.current_col, cursor.current_row).first;
  
  text_selected.selected = false;
}

void PieceTable::copy(Cursor& cursor) {
  std::cout << "Copy function" << std::endl;
  int anchor_point_pos_in_buffer = findNewCursorPos(0, text_selected.anchor_point.x, text_selected.anchor_point.y).first;
  int active_point_pos_in_buffer = findNewCursorPos(0, text_selected.active_point.x, text_selected.active_point.y).first;

  size_t selection_start = std::min(anchor_point_pos_in_buffer, active_point_pos_in_buffer);
  size_t selection_end = std::max(anchor_point_pos_in_buffer, active_point_pos_in_buffer);
  
  size_t offset = 0;
  std::string string_copied;

  for (const auto& piece : pieces) {
    size_t piece_end_offset = offset+piece->length;

    if (offset <= selection_start && piece_end_offset >= selection_end) { // selection inside the piece
      std::string substring = piece->buffer->substr(piece->start, piece->length);
      size_t length_to_copy = selection_end - selection_start;
      size_t start_string_to_copy = selection_start-offset;
      
      std::string substring_to_copy = substring.substr(start_string_to_copy, length_to_copy);
      string_copied += substring_to_copy;
    }
    else if (offset <= selection_start && piece_end_offset > selection_start) {// select from the end
      std::string substring = piece->buffer->substr(piece->start, piece->length);
      size_t length_to_copy = piece_end_offset - selection_start;
      
      std::string substring_to_copy = substring.substr(substring.length()-length_to_copy, length_to_copy);
      string_copied += substring_to_copy;
    }
    else if (offset < selection_end && piece_end_offset >= selection_end) { // select from the start
      size_t length_to_copy = selection_end - offset;
      std::string substring = piece->buffer->substr(piece->start, length_to_copy);
      
      string_copied += substring;

    }
    else if (offset > selection_start && piece_end_offset < selection_end) { // copy entirely
      std::string substring = piece->buffer->substr(piece->start, piece->length);
      string_copied += substring;
    }

    offset += piece->length;
  }

  SetClipboardText(string_copied.c_str());
}

void PieceTable::paste(Cursor& cursor) {
  if (GetClipboardText()) {
    std::string text_to_paste = GetClipboardText();
    size_t length_text_to_paste = text_to_paste.length();
    
    if (text_selected.selected) {
      deleteFromSelection();
      unselectText(cursor);
      updateRowSize(0, cursor.current_row, cursor.current_col);
    }
    updatePieces(text_to_paste, cursor.current_pos, true, cursor.current_col);
    cursor.current_pos += length_text_to_paste;
    cursor.current_col += length_text_to_paste;
    cursor.cursor_moved = true;
    updateRowSize(0, cursor.current_row, cursor.current_col);
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

  // free undo_stack
  for (auto& action_redo : redo_stack) {
    delete action_redo;
  }
}
