CC=gcc


judge: judge.c
	$(CC) judge.c -o judge

test: test.c
	$(CC) test.c -o test

judge_test: judge test
	./judge ./test 100 2048 1.in 1.tmp.out
