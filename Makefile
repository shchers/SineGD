CC := gcc
LIBS := -lgd -lm -lfftw3
CFLAGS := -std=gnu99 -Wall
BIN := testgd

all:
	@$(CC) testgd.c $(CFLAGS) $(LIBS) -o $(BIN)

run:
	@./$(BIN) | tee $(BIN).log

show:
	@firefox $(BIN).png

clean:
	@rm -f $(BIN) $(BIN).png $(BIN).log
