
OUT_DIR=./out
SHARED_OUT_DIR=./shared
IDIR=./src
TEST_DIR = ./tmp


MKDIRS += out
MKDIRS += shared
MKDIRS += tmp

CFLAGS= -Wall -pthread

ifdef DEBUG
CFLAGS += -g -DDEBUG
else
CFLAGS += -O3
endif

_DEPS = child.h cli.h constants.h diff.h run.h utils.h log.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = main.o child.o cli.o diff.o run.o utils.o log.o
OBJ = $(patsubst %,$(OUT_DIR)/%,$(_OBJ))

RELEASE := $(shell cat /etc/os-release | grep NAME | awk 'NR==1' | cut -d '=' -f 2 | sed 's/\"//g')
ifeq ($(RELEASE), Alpine Linux)
OBJ += /usr/lib/libargp.a
endif

$(OUT_DIR)/%.o: $(IDIR)/%.c $(DEPS) | $(OUT_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

runner: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

S_OBJ = $(patsubst %,$(SHARED_OUT_DIR)/%,$(_OBJ))

$(SHARED_OUT_DIR)/%.o: $(IDIR)/%.c $(DEPS) | $(SHARED_OUT_DIR)
	$(CC) $(CFLAGS) -c -fPIC -o $@ $<

librunner: $(S_OBJ)
	$(CC) $(CFLAGS) -shared -o $@.so $^

.PHONY: clean
clean:
	rm -f $(OUT_DIR)/*.o $(SHARED_OUT_DIR)/*.o *~ $(IDIR)/*~


C_BASE=./tests/c
NODE_BASE=./tests/node

c: $(C_BASE)/main.c
	$(CC) $< -o main

testc: c runner | $(TEST_DIR)
	./runner -l $(TEST_DIR)/c.log -t 1000 -m 2048 -i $(C_BASE)/1.in -o $(C_BASE)/1.out -u $(TEST_DIR)/c.tmp.out ./main

testnode: runner | $(TEST_DIR)
	./runner -l $(TEST_DIR)/node.log -t 1000 -m 2048 --mco -i $(NODE_BASE)/1.in -o $(NODE_BASE)/1.out -u $(TEST_DIR)/node.tmp.out -- node $(NODE_BASE)/main.js


cleantest:
	rm -f $(TEST_DIR) main

$(sort $(MKDIRS)):
	mkdir -p $@
