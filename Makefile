# ============================================================
#  Reed–Solomon Codec (fec-rs-codec)
#  Build settings
# ============================================================

CC      = gcc
CFLAGS  = -O2 -Wall -std=c99 -Iinclude
LDFLAGS = -lm

# ------------------------------------------------------------
# Source files
# ------------------------------------------------------------
SRC = \
    src/rs_gf.c \
    src/rs_encoder.c \
    src/rs_decoder.c

OBJ = $(SRC:.c=.o)

TEST_SRC = mains/rs_ber_bler.c
TEST_OBJ = $(TEST_SRC:.c=.o)

BIN_DIR = bin
TARGET_NAME = rs_ber_bler

# OS によって拡張子を切り替え
ifeq ($(OS),Windows_NT)
    TARGET = $(BIN_DIR)/$(TARGET_NAME).exe
else
    TARGET = $(BIN_DIR)/$(TARGET_NAME)
endif

# ============================================================
#  Default build target
# ============================================================
all: $(TARGET)

# Create bin/ directory (Linux + macOS + Windows WSL 対応)
$(BIN_DIR):
	@if [ ! -d "$(BIN_DIR)" ]; then \
		mkdir -p $(BIN_DIR) 2>/dev/null || mkdir $(BIN_DIR); \
	fi

# Link
$(TARGET): $(BIN_DIR) $(OBJ) $(TEST_OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(TEST_OBJ) $(LDFLAGS)

# Compile
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# ============================================================
#  Run
# ============================================================
run: $(TARGET)
	./$(TARGET)

# ============================================================
#  Clean (Windows + Linux/macOS 完全対応)
# ============================================================
clean:
	@echo "Cleaning object files..."
	rm -f $(OBJ) $(TEST_OBJ)

	@echo "Cleaning binaries..."
	# Windows
	@if [ -f "$(BIN_DIR)/$(TARGET_NAME).exe" ]; then \
		rm -f "$(BIN_DIR)/$(TARGET_NAME).exe"; \
	fi
	# Linux/macOS
	@if [ -f "$(BIN_DIR)/$(TARGET_NAME)" ]; then \
		rm -f "$(BIN_DIR)/$(TARGET_NAME)"; \
	fi

	# Remove bin/ if empty
	@if [ -d "$(BIN_DIR)" ] && [ ! "$$(ls -A $(BIN_DIR))" ]; then \
		echo "Removing empty bin directory"; \
		rmdir $(BIN_DIR); \
	fi

.PHONY: all clean run
