APP = $(PWD)
BIN = $(APP)/bin

INCL = -I $(APP)/include/ -I $(APP)/third_party/
AR = $(APP)/lib/libgflags.a
LIB_PATH = -L /usr/local/lib
LIBS = -lpthread

CC = /usr/gcc-4.8.1/bin/g++
CXXFLAGS = -std=c++11 -fPIC -Wall -static-libstdc++ -march=native -O3

EXE = $(BIN)/fm
OBJS = src/main.o \
       src/parser.o \
       src/learner.o \
       src/lr_learner.o \
       src/fm_adag_learner.o \
       src/fm_ftrl_learner.o \
       src/ffm_adag_learner.o \
       src/ffm_ftrl_learner.o \
       src/fxx_adag_learner.o \
       src/fxxh_adag_learner.o \
       src/auc_learner.o


.PHONY : all clean

all: $(EXE)

.cc.o:
	$(CC) $(CXXFLAGS) -c $< -o $@ $(INCL)

$(EXE): $(OBJS)
	$(CC) $(CXXFLAGS) $^ -o $@ $(INCL) $(AR) $(LIB_PATH) $(LIBS) 

clean:
	rm -rf $(EXE) $(OBJS)
