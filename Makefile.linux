CIBLE = main
SRCS =  Main.cpp 
LIBS =  -lglut -lGLU -lGL -lm 

CC = g++
CPP = g++

FLAGS = -Wall -O2 

CFLAGS = $(FLAGS)
CXXFLAGS = $(FLAGS)

OBJS = $(SRCS:.cpp=.o)   

$(CIBLE): $(OBJS)
	g++ $(LDFLAGS) -o $(CIBLE) $(OBJS) $(LIBS)
clean:
	rm -f  *~  $(CIBLE) $(OBJS)

Main.o: Main.cpp



