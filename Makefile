
OUT_DIR=./out
SHARED_OUT_DIR=./shared
IDIR=./src

MKDIRS += out
MKDIRS += shared

CFLAGS= -Wall -std=gnu17

ifdef DEBUG
CFLAGS += -g -DDEBUG
else
CFLAGS += -O3
endif

_DEPS = cli.h constants.h diff.h run.h utils.h log.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = judge.o cli.o diff.o run.o utils.o log.o
OBJ = $(patsubst %,$(OUT_DIR)/%,$(_OBJ))

$(OUT_DIR)/%.o: $(IDIR)/%.c $(DEPS) | $(OUT_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

judge: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

S_OBJ = $(patsubst %,$(SHARED_OUT_DIR)/%,$(_OBJ))

$(SHARED_OUT_DIR)/%.o: $(IDIR)/%.c $(DEPS) | $(SHARED_OUT_DIR)
	$(CC) $(CFLAGS) -c -fPIC -o $@ $<

libjudge: $(S_OBJ)
	$(CC) $(CFLAGS) -shared -o $@.so $^

.PHONY: clean
clean:
	rm -f $(OUT_DIR)/*.o $(SHARED_OUT_DIR)/*.o *~ $(IDIR)/*~

ifdef TEST
BASE=./tests/$(TEST)

$(TEST): $(BASE)/$(TEST).c
	$(CC) $< -o main

test: $(TEST) judge
	./judge judge -l $<.log ./main 1000 2048 $(BASE)/$<.in $(BASE)/$<.out $<.tmp.out

testr: $(TEST) judge
	./judge run -l $<.log ./main 1000 2048 $(BASE)/$<.in $<.tmp.out

testc: $(TEST) judge
	./judge check -l $<.log $(BASE)/$<.out $<.tmp.out

cleantest:
	rm -f $(TEST).log $(TEST).tmp.out main
endif


$(sort $(MKDIRS)):
	mkdir -p $@
