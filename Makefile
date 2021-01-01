CC=gcc
OUT_O_DIR=./out
SHARED_OUT_DIR=./shared
IDIR=./src
CFLAGS= -Wall -lm -g

MKDIRS += out
MKDIRS += shared

_DEPS = cli.h constants.h diff.h run.h utils.h log.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = judge.o cli.o diff.o run.o utils.o log.o
OBJ = $(patsubst %,$(OUT_O_DIR)/%,$(_OBJ))

$(OUT_O_DIR)/%.o: $(IDIR)/%.c $(DEPS) | $(OUT_O_DIR)
	$(CC) $(CFLAGS) -c -o $@ $< $(CFLAGS)

judge: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

S_OBJ = $(patsubst %,$(SHARED_OUT_DIR)/%,$(_OBJ))

$(SHARED_OUT_DIR)/%.o: $(IDIR)/%.c $(DEPS) | $(SHARED_OUT_DIR)
	$(CC) $(CFLAGS) -c -fPIC -o $@ $< $(CFLAGS)

.PHONY: libjudge
libjudge: $(S_OBJ)
	$(CC) $(CFLAGS) -shared -o libjudge.so $^

.PHONY: clean
clean:
	rm -f $(OUT_O_DIR)/*.o $(SHARED_OUT_DIR)/*.o *~ $(IDIR)/*~

TESTS=./tests
1: $(TESTS)/1/1.c
	$(CC) $< -o main

test1: 1 judge
	./judge judge -l 1.log ./main 1000 2048 $(TESTS)/$</$<.in $(TESTS)/$</$<.out 1.tmp.out

test1r: 1 judge
	./judge run -l 1.log ./main 1000 2048 $(TESTS)/$</$<.in 1.tmp.out

test1c: 1 judge
	./judge check -l 1.log $(TESTS)/$</$<.out 1.tmp.out

cleantest1:
	rm -f 1.log 1.tmp.out main

$(sort $(MKDIRS)):
	mkdir -p $@
