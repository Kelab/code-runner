CC=gcc
MKDIRS += out
OUT_O_DIR=./out
IDIR=./src
CFLAGS= -Wall

_DEPS = constants.h diff.h run.h utils.h log.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = judge.o utils.o diff.o run.o log.o
OBJ = $(patsubst %,$(OUT_O_DIR)/%,$(_OBJ))

$(OUT_O_DIR)/%.o: $(IDIR)/%.c $(DEPS) | $(OUT_O_DIR)
	$(CC) -c -o $@ $< $(CFLAGS)

judge: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean
clean:
	rm -f $(OUT_O_DIR)/*.o *~ $(IDIR)/*~ main

TESTS=./tests
1: $(TESTS)/1/1.c
	$(CC) $< -o main

test1: 1 judge
	./judge ./main 1000 2048 $(TESTS)/$</$<.in $(TESTS)/$</$<.out 1.tmp.out 1.log


$(sort $(MKDIRS)):
	mkdir -p $@
