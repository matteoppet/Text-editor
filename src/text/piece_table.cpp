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

void PieceTable::updatePieces(const char char_to_insert, size_t position_to_insert, bool insert_new_piece) {
  addBuffer += char_to_insert;

  size_t offset = 0;
  std::vector<Piece*> new_pieces;

  for (auto piece : pieces) {
    // * case 1: split pieces
    if (offset < position_to_insert && offset+piece->length > position_to_insert) {
      size_t local_offset = position_to_insert-offset;

      Piece* first_part_piece = new Piece({piece->fromAddBuffer, &addBuffer, piece->start, local_offset});
      new_pieces.push_back(first_part_piece);

      Piece* new_part_piece = new Piece({true, &addBuffer, addBuffer.length()-1, 1});
      new_pieces.push_back(new_part_piece);

      Piece* last_part_piece = new Piece({piece->fromAddBuffer, &addBuffer, piece->start+local_offset, piece->length-local_offset});
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

void PieceTable::deleteChar(size_t position_to_delete) {
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
      
      removed = true;

    } else if (offset+piece->length == cursor_pos_before_deletion && !removed) {   
      if (piece->length > 1) {
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

void PieceTable::renderPieces() {
  float start_x = 0;
  float start_y = 0;
  float length_current_word = 0;
  FontClass& instance_font_class = FontClass::getInstance();
  
  // draw pieces
  for (const auto& piece : pieces) {
    std::string substring = piece->buffer->substr(piece->start, piece->length);

    if (substring.compare("\n") == 0) {
      start_y += 18;
      start_x = 0;
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