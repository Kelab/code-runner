CC=gcc
MKDIRS += out
OUT_O_DIR=./out
LDIR=./lib
IDIR=./src
CFLAGS=-I$(IDIR) -Wall

LIBS=-lm

_DEPS = constants.h diff.h run.h utils.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = judge.o utils.o diff.o run.o
OBJ = $(patsubst %,$(OUT_O_DIR)/%,$(_OBJ))


$(OUT_O_DIR)/%.o: $(IDIR)/%.c $(DEPS) | $(OUT_O_DIR)
	$(CC) -c -o $@ $< $(CFLAGS)

judge: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(OUT_O_DIR)/*.o *~ $(IDIR)/*~

TESTS=./tests
1: $(TESTS)/1/1.c
	$(CC) $< -o test

test1: 1 judge
	./judge ./test 1000 2048 $(TESTS)/$</$<.in $<.tmp.out $<.result


$(sort $(MKDIRS)):
	mkdir -p $@
