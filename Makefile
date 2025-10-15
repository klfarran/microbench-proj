CFLAGS := -O0 -march=native -Wall -Wextra -Iinclude -w 
LDFLAGS := -lpthread
BIN := build
SRCS := $(wildcard src/*.c)
TARGET := $(BIN)/harness

all: $(TARGET)

$(TARGET): | $(BIN)
	$(CC) $(CFLAGS) $(SRCS) -o $@ $(LDFLAGS)
	
$(BIN):
	mkdir -p $(BIN)

clean:
	rm -rf $(BIN)
