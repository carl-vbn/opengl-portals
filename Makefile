CXX := g++

OBJ_PATH := obj
SRC_PATH := src
INCLUDE_PATH := include
TARGET := program

CXXFLAGS := -g -Wall -I$(INCLUDE_PATH)
LDFLAGS :=
LDLIBS := -lglfw

SRC := $(foreach x, $(SRC_PATH), $(wildcard $(addprefix $(x)/*,.c*)))
OBJ := $(addprefix $(OBJ_PATH)/, $(addsuffix .o, $(notdir $(basename $(SRC)))))

default: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(LDFLAGS) $(OBJ) $(LDLIBS) -o $@

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.c* $(INCLUDE_PATH)/%.h
	@mkdir -p $(OBJ_PATH)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.c*
	@mkdir -p $(OBJ_PATH)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

.PHONY: clean
clean:
	rm -rf $(OBJ)
	rm -f $(TARGET)

.PHONY: run
run: default
	./$(TARGET)

