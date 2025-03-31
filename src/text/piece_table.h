#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <bits/stdc++.h>

#include "../font/font.h"

struct Piece {
  bool fromAddBuffer;
  const std::string* buffer;
  size_t start;
  size_t length;
};

class PieceTable {
  private:
    std::string originalBuffer;
    std::string addBuffer;
    std::vector <Piece*> pieces;
    std::map<int,int> size_rows;

  public:
    void updateOriginalBuffer(const std::string& text);
    void updatePieces(const char char_to_insert, size_t position_to_insert, bool insert_new_piece);
    void deleteChar(size_t position_to_delete);
    void renderPieces();
    void insertNewRow(size_t cursor_pos);

    void updateRowSize(int action, int cursor_row, int cursor_col);
    int getRowSize(int row);
    int getTotalRows();
    
    std::pair<int, int> findNewCursorPos(int action, int cursor_col, int cursor_row);
};