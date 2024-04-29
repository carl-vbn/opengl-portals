CXX := g++
CXXFLAGS :=
LDFLAGS := -lglfw

OBJ_PATH := obj
SRC_PATH := src

SRC := $(foreach x, $(SRC_PATH), $(wildcard $(addprefix $(x)/*,.c*)))
OBJ := $(addprefix $(OBJ_PATH)/, $(addsuffix .o, $(notdir $(basename $(SRC)))))

default: program

program: $(OBJ)
	$(CXX) -o $@ $(OBJ) $(LDFLAGS)

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.c*
	$(CXX) $(CCXFLAGS) -c -o $@ $<

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

