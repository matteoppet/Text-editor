CXX = g++
OUTPUT_FILE = editor.exe
SRCs = src/main.cpp src/text/piece_table.cpp src/font/font.cpp src/cursor/cursor.cpp src/utils/utils.cpp
FLAGS = -O1 -Wall -Wno-missing-braces -Iinclude/ -Llib/ -lraylib -lopengl32 -lgdi32 -lwinmm

main: src/main.cpp
	$(CXX) $(SRCs) -o $(OUTPUT_FILE) $(FLAGS)