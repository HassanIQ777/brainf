# ============================================================
#  Brainf — Static Builder
# ============================================================

CXX      := gcc
CXXFLAGS := -std=c17 -Wall -Wextra -Iinclude
LDFLAGS  := 
AR       := ar
ARFLAGS  := rcs

SRC_DIR  := src
OBJ_DIR  := obj
LIB_DIR  := lib
BIN_DIR  := bin
LIB      := $(LIB_DIR)/libbrainf.a

SRCS     := $(wildcard $(SRC_DIR)/*.c)
OBJS     := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

# --- run target: first .c at root ---
# Override with: make run SRC=main.c
SRC      := $(wildcard *.c)
RUN_BIN  := $(BIN_DIR)/$(basename $(SRC))

# ============================================================
#  Build modes
# ============================================================
ifeq ($(MAKECMDGOALS), debug)
    CXXFLAGS += -g -O0 -DDEBUG -fsanitize=address,undefined
    LDFLAGS  += -fsanitize=address,undefined
    $(info [Brainf] Building in DEBUG mode)
else
    CXXFLAGS += -O2 -DNDEBUG
    $(info [Brainf] Building in RELEASE mode)
endif

# ============================================================
#  Rules
# ============================================================

all: dirs $(LIB)
debug: dirs $(LIB)

$(LIB): $(OBJS)
	@echo "[AR]  $@"
	@$(AR) $(ARFLAGS) $@ $^
	@echo "[OK]  libbrainf.a sealed."

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@echo "[CC]  $<"
	@$(CXX) $(CXXFLAGS) -c $< -o $@

run: all
	$(if $(SRC),,$(error No runnable .c found at root. Use: make run SRC=yourfile.c))
	@echo "[CC]  $(SRC) → $(RUN_BIN)"
	@$(CXX) $(CXXFLAGS) $(SRC) -o $(RUN_BIN) $(LDFLAGS)
	@echo "[RUN] $(RUN_BIN)"
	@./$(RUN_BIN)

dirs:
	@mkdir -p $(OBJ_DIR) $(LIB_DIR) $(BIN_DIR)

clean:
	@rm -rf $(OBJ_DIR) $(LIB_DIR) $(BIN_DIR)
	@echo "[CLN] Library dust swept away."

rebuild: clean all

info:
	@echo "Sources : $(SRCS)"
	@echo "Objects : $(OBJS)"
	@echo "Library : $(LIB)"
	@echo "Run src : $(SRC)"
	@echo "Run bin : $(RUN_BIN)"

.PHONY: all debug run clean rebuild info dirs