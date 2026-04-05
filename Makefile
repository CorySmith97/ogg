CC=clang
C_FILES=src/main.c
M_FILES=src/main.m
C_FLAGS=-std=c23 -Wall -g -Wno-missing-braces
C_FLAGS+= -Wextra -Wno-unused-function -fno-strict-aliasing
C_FLAGS+= -Wno-unused-variable -Wno-unused-parameter
C_FLAGS+= -DPLATFORM_SDL
C_FLAGS+= -DDEBUG
SAN= -fsanitize=address,undefined
BUILD_DIR=bin

INCLUDE=
INCLUDE+= -I/opt/homebrew/Cellar/sdl2/2.32.10/include/SDL2
INCLUDE+= -I/opt/homebrew/Cellar/sdl2_image/2.8.8/include/SDL2

LINK=
LINK+= -L/opt/homebrew/Cellar/sdl2/2.32.10/lib -lsdl2
LINK+= -L/opt/homebrew/Cellar/sdl2_image/2.8.8/lib -lsdl2_image

.SILENT:

unity:
	$(CC) -o $(BUILD_DIR)/app $(C_FILES) $(C_FLAGS) $(LINK) $(INCLUDE)

sana:
	$(CC) -o $(BUILD_DIR)/app $(C_FILES) $(C_FLAGS) $(SAN) $(LINK) $(INCLUDE)
