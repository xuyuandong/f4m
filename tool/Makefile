APP = $(PWD)/..
BIN = $(APP)/bin

INCL = -I $(APP)/third_party/
AR = $(APP)/lib/libgflags.a 
#LIB_PATH = -L /usr/local/lib
#LIBS = -lpthread

CC = /usr/gcc-4.8.1/bin/g++
CXXFLAGS = -std=c++11 -fPIC -Wall -static-libstdc++ -O3

EXE = $(BIN)/en
OBJS = main.o \
       encoder.o 


.PHONY : all clean

all: $(EXE)

.cc.o:
	$(CC) $(CXXFLAGS) -c $< -o $@ $(INCL)

$(EXE): $(OBJS)
	$(CC) $(CXXFLAGS) $^ -o $@ $(INCL) $(AR) #$(LIB_PATH) $(LIBS) 

clean:
	rm -rf $(EXE) $(OBJS)
