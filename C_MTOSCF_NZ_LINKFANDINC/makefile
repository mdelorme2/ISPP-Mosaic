GUROBI_DIR = /home/mdelorme/gurobi1303/linux64
CLIB     = -L$(GUROBI_DIR)/lib/ -lgurobi130
INC      = $(GUROBI_DIR)/include/
CPPLIB   = -L$(GUROBI_DIR)/lib/ -lgurobi_c++ $(CLIB)

CLIPPER_SRC = /home/mdelorme/1_ISPP/_Clipper2/Clipper2Lib/src
CLIPPER_INC = /home/mdelorme/1_ISPP/_Clipper2/Clipper2Lib/include/
CLIPPER_OBJS = $(CLIPPER_SRC)/clipper.engine.o \
               $(CLIPPER_SRC)/clipper.offset.o \
               $(CLIPPER_SRC)/clipper.rectclip.o 

GUROBI_OPTS = -I$(INC) -I$(CLIPPER_INC) $(CPPLIB) -lpthread -lm -m64 

CC = g++ 
CFLAGS = -O2 -Wall -pedantic -DIL_STD -std=c++17
DEBUG = -pg -g -Wall -pedantic -DIL_STD 
OBJECTS = main.o helper_functions_part1.o helper_functions_part2.o helper_functions_part3.o time.o $(CLIPPER_OBJS)
 
exec : $(OBJECTS) 
	$(CC) $(CFLAGS) -o ILP $(OBJECTS) $(GUROBI_OPTS)
 
.cpp.o : 
	$(CC) $(CFLAGS) $(GUROBI_OPTS) -c $< -o $@ 
 
clean : 
	rm -f $(OBJECTS) ILP 