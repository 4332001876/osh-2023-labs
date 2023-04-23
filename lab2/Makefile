CC=g++
CFLAGS=-c -g -Wall
SOURCES=shell.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=shell

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@

.cpp.o:   #。cpp->.o 后缀规则
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f *o $(EXECUTABLE)