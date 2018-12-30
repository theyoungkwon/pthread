# define the C compiler to use
CC = gcc
CXX = g++
# define any compile-time flags
CFLAGS = -Wall -g
CXXFLAGS = -std=c++11
RM = rm -f

# define the C source files
SRCS = sugar.c queue.c lcgrand.c

# define the C object files
OBJS = $(SRCS:.c=.o)
HEADERS = $(SRCS:.c=.h)
# define the executable file
TARGET = sugar

all:    $(TARGET)
	@echo  Simple compiler named main has been compiled

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) -lpthread

sugar.o : sugar.c $(HEADERS)
	$(CC) $(CFLAGS) -c sugar.c $(HEADERS)

queue.o : queue.c queue.h
	$(CC) $(CFLAGS) -c queue.c queue.h

lcgrand.o : lcgrand.c lcgrand.h
	$(CC) $(CFLAGS) -c lcgrand.c lcgrand.h

clean:
	rm *.o
	rm *.gch