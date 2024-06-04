CXX := g++

OBJ_PATH := obj
SRC_PATH := src
INCLUDE_PATH := include

CXXFLAGS := -g -Wall -I$(INCLUDE_PATH)
LDFLAGS :=
LDLIBS := -lglfw

SRC := $(foreach x, $(SRC_PATH), $(wildcard $(addprefix $(x)/*,.c*)))
OBJ := $(addprefix $(OBJ_PATH)/, $(addsuffix .o, $(notdir $(basename $(SRC)))))

default: program

program: $(OBJ)
	$(CXX) $(LDFLAGS) $(OBJ) $(LDLIBS) -o $@

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.c* $(INCLUDE_PATH)/%.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.c*
	$(CXX) $(CXXFLAGS) -c -o $@ $<

.PHONY: makedir
makedir:
	mkdir $(OBJ_PATH)

.PHONY: clean
clean:
	rm -rf $(OBJ)
	rm -f program

.PHONY: run
run: default
	./program

