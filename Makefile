CXX = g++

OUTPUT_FILE = editor.exe
SRC_DIR = src

SRCs = $(wildcard $(SRC_DIR)/**/*.cpp) $(SRC_DIR)/main.cpp include/tinyfiledialogs.c

FLAGS = -O1 -Wall -Wno-missing-braces -Iinclude/ -Llib/ -lraylib -lopengl32 -lgdi32 -lwinmm -lcomdlg32 -lole32
main: src/main.cpp
	$(CXX) $(SRCs) -o $(OUTPUT_FILE) $(FLAGS)