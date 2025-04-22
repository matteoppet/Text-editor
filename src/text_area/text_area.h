#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <bits/stdc++.h>
#include <fstream>

#include "../font/font.h"
#include "cursor.h"

#include <memory>

enum class ActionType {
  DELETE,
  SPLIT,
  INSERT,
};

struct Piece {
  bool fromAddBuffer;
  const std::string* buffer;
  size_t start;
  size_t length;

  Piece(bool fromAddBuffer, const std::string* buffer, size_t start, size_t length) : 
    fromAddBuffer(fromAddBuffer), 
    buffer(buffer), 
    start(start), 
    length(length) {}
};

struct TextSelectedInfo {
  bool selected = false; 
  Vector2 anchor_point;
  Vector2 active_point;
};

struct ActionRecordUndo {
  ActionType type;

  size_t index_in_piece;
  size_t cursor_col;

  Piece* piece_referred;
  Piece* piece_before_splitting; // * optional
  Piece* piece_before_deletion; // * optional

  ActionRecordUndo(ActionType type, size_t index_in_piece, size_t cursor_col, Piece* piece_referred, Piece* piece_before_splitting, Piece* piece_before_deletion) :
    type(type), 
    index_in_piece(index_in_piece), 
    cursor_col(cursor_col),
    piece_referred(piece_referred), 
    piece_before_splitting(piece_before_splitting), 
    piece_before_deletion(piece_before_deletion) {}
};

struct ActionRecordRedo {
  ActionType type;

  size_t index_in_piece;
  size_t cursor_col;

  std::vector<Piece*> piece_splitted;
  std::vector<Piece*> piece_deleted;
  Piece* piece_decreased_length;
  Piece* piece_to_insert = nullptr;
  ActionRecordUndo* undo_action;

  ActionRecordRedo(ActionType type, size_t index_in_piece, size_t cursor_col, std::vector<Piece*> piece_splitted, std::vector<Piece*> piece_deleted, Piece* piece_decreased_length, ActionRecordUndo* undo_action) :
  type(type),
  index_in_piece(index_in_piece),
  cursor_col(cursor_col),
  piece_splitted(piece_splitted),
  piece_deleted(piece_deleted),
  piece_decreased_length(piece_decreased_length),
  undo_action(undo_action) {}
};

class PieceTable {
  private:
    std::string originalBuffer;
    std::string addBuffer;
    std::vector<Piece*> pieces;
    std::map<int,int> size_rows;

    std::vector<ActionRecordUndo*> undo_stack;
    std::vector<ActionRecordRedo*> redo_stack;

    int mouse_scroll_new_position = 0;
    int mouse_scroll_speed = 8;
    
  public:
    TextSelectedInfo text_selected;

    void updateOriginalBuffer(const std::string& text);
    void updatePieces(std::string char_to_insert, size_t position_to_insert, bool insert_new_piece, size_t cursor_col);
    void deleteChar(size_t position_to_delete, size_t cursor_col);
    void deleteFromSelection();

    void render(Cursor& cursor);

    void updateRowSize(int action, int cursor_row, int cursor_col);
    int getRowSize(int row);
    int getTotalRows();
    
    std::pair<int, int> findNewCursorPos(int action, int cursor_col, int cursor_row);
    void readDroppedFile(std::string file_path, Cursor& cursor);
    void reset(); 
    void saveFile(std::string file_path);

    void undo(Cursor& cursor);
    void redo(Cursor& cursor);

    void selectText(Cursor& cursor, std::string direction);
    void unselectText(Cursor& cursor);

    void copy(Cursor& cursor);
    void paste(Cursor& cursor);

    void freeMemory();
};