CXX = g++
CXXFLAGS = -Wall
LDFLAGS = -L. -lgc -lreadline 

ifdef NDEBUG
	CXXFLAGS += -O3
else
	CXXFLAGS += -g3 -fno-inline
endif

SRC = core.cc scan.cc parse.cc prim.cc main.cc
OBJ = ${SRC:.cc=.o}

all: mecano

mecano: $(OBJ)
	@echo "[L] mecano"
	@$(CXX) -o mecano $(OBJ) $(LDFLAGS) $(CXXFLAGS) -lgc -lstdc++

$(OBJ): core.hh
scan.o: scan.hh

.cc.o:
	@echo "[C]" $<
	@$(CXX) -c -o $@ $< $(CXXFLAGS)

clean: 
	@rm -f $(OBJ) mecano
	@echo "ok"

todo:
	@grep -rInso 'TODO: \(.\+\)' *.cc *.hh test/*.cc || true

.PHONY: all
