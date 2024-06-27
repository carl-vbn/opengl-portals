CXX := g++

OBJ_PATH := obj
SRC_PATH := src
INCLUDE_PATH := include

ifeq ($(OS),Windows_NT)
TARGET := program.exe
else
TARGET := program
endif

CXXFLAGS := -g -Wall -I$(INCLUDE_PATH) -std=c++11
LDFLAGS :=
LDLIBS := -lglfw

SRC := $(foreach x, $(SRC_PATH), $(wildcard $(addprefix $(x)/*,.c*)))
OBJ := $(addprefix $(OBJ_PATH)/, $(addsuffix .o, $(notdir $(basename $(SRC)))))

default: $(TARGET)

ifeq ($(OS),Windows_NT)
$(TARGET): $(OBJ)
	$(CXX) $(LDFLAGS) $(OBJ) glfw3.dll -o $@
else
$(TARGET): $(OBJ)
	$(CXX) $(LDFLAGS) $(OBJ) $(LDLIBS) -o $@
endif

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.c* $(INCLUDE_PATH)/%.h
	@make -s mkdir
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.c*
	@make -s mkdir
	$(CXX) $(CXXFLAGS) -c -o $@ $<

.PHONY: mkdir
ifeq ($(OS),Windows_NT)
mkdir:
	@if not exist $(OBJ_PATH) mkdir $(OBJ_PATH)
else
mkdir:
	@mkdir -p $(OBJ_PATH)
endif

.PHONY: clean
ifeq ($(OS),Windows_NT)
clean:
	if exist $(OBJ_PATH) del $(OBJ_PATH)\* /s /q
	if exist $(TARGET) del $(TARGET)
else
clean:
	rm -rf $(OBJ)
	rm -f $(TARGET)
endif

.PHONY: run
ifeq ($(OS),Windows_NT)
run: default
	$(TARGET)
else
run: default
	./$(TARGET)	
endif