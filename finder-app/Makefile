CC=$(CROSS_COMPILE)gcc
LD=$(CROSS_COMPILE)ld

RM := rm -f

BIN	:=	writer

SRC_PATH =	./
SRC :=	writer.c
OBJ := 	writer.o

all: $(BIN)

$(BIN): $(OBJ) 
	@$(CC) -o $(BIN) $(OBJ)

clean:
	@$(RM) $(OBJ) $(BIN)

%.o: %.c
	@$(CC) -c $< -o $@