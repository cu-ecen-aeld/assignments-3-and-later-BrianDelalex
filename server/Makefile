CC = $(CROSS_COMPILE)gcc

RM := rm -f

BIN := aesdsocket

INCLUDES := ./inc

CFLAGS += -g -Wall -Werror

SRC := ./src/aesdsocket.c
OBJ := ./src/aesdsocket.o

ALL: $(BIN)

$(BIN): $(OBJ)
	@$(CC) -o $(BIN) $(OBJ) $(CFLAGS)

clean:
	@$(RM) $(OBJ) $(BIN)

%o: %c
	@$(CC) -I$(INCLUDES) -c $< -o $@ $(CFLAGS)