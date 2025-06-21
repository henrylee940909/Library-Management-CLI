CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic -g

INC_DIR  = include
SRC_DIR  = src
OBJ_DIR  = obj
BIN_DIR  = bin
DATA_DIR = data

ifeq ($(OS),Windows_NT)
    EXE  = .exe
    MKDIR = if not exist $(1) mkdir $(1)
    RM    = del /Q
else
    EXE  =
    MKDIR = mkdir -p $(1)
    RM    = rm -rf
endif

SOURCES  := $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS  := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SOURCES))
TARGET   := $(BIN_DIR)/library_manager$(EXE)

dirs:
	@$(call MKDIR,$(OBJ_DIR))
	@$(call MKDIR,$(BIN_DIR))
	@$(call MKDIR,$(DATA_DIR))

all: dirs $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | dirs
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) -c $< -o $@

clean:
ifeq ($(OS),Windows_NT)
	-$(RM) $(OBJ_DIR)\*.o
	-$(RM) $(TARGET)
else
	$(RM) $(OBJ_DIR)/*.o $(TARGET)
endif

run: $(TARGET)
ifeq ($(OS),Windows_NT)
	$(TARGET)
else
	./$(TARGET)
endif

.PHONY: all clean run dirs
