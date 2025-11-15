CC      = gcc
CFLAGS  = -O2 -Wall -std=c99 -Iinclude
LDFLAGS = -lm

SRC = \
    src/nsc_encoder.c \
    src/nsc_decoder.c \
    src/trellis.c

OBJ = $(SRC:.c=.o)

TEST_SRC = mains/nsc_ber.c
TEST_OBJ = $(TEST_SRC:.c=.o)

BIN_DIR = bin
TARGET_NAME = nsc_ber

# OS によって実行ファイル名を切り替え
ifeq ($(OS),Windows_NT)
    TARGET = $(BIN_DIR)/$(TARGET_NAME).exe
else
    TARGET = $(BIN_DIR)/$(TARGET_NAME)
endif

# ============================================================
#  Build rules
# ============================================================

all: $(TARGET)

# Create bin directory (cross-platform)
$(BIN_DIR):
	@if [ ! -d "$(BIN_DIR)" ]; then \
		mkdir -p $(BIN_DIR) 2>/dev/null || mkdir $(BIN_DIR); \
	fi

$(TARGET): $(BIN_DIR) $(OBJ) $(TEST_OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(TEST_OBJ) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# ============================================================
#  Run
# ============================================================
run: $(TARGET)
	./$(TARGET)

# ============================================================
#  Clean (Windows + Linux 完全対応)
# ============================================================
clean:
	@echo "Cleaning object files..."
	rm -f $(OBJ) $(TEST_OBJ)

	@echo "Cleaning binaries..."
	# Windows .exe 削除
	@if [ -f "$(BIN_DIR)/$(TARGET_NAME).exe" ]; then rm -f "$(BIN_DIR)/$(TARGET_NAME).exe"; fi

	# Linux/macOS バイナリ削除
	@if [ -f "$(BIN_DIR)/$(TARGET_NAME)" ]; then rm -f "$(BIN_DIR)/$(TARGET_NAME)"; fi

	# bin フォルダ内が空なら削除
	@if [ -d "$(BIN_DIR)" ] && [ ! "$$(ls -A $(BIN_DIR))" ]; then \
		echo "Removing empty bin directory"; \
		rmdir $(BIN_DIR); \
	fi

.PHONY: all clean run
