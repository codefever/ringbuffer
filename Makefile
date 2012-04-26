LIB_NAME:=librbuf.so
TEST_BIN:=rbtest
CC:=gcc

all:	$(LIB_NAME) test
test:	$(TEST_BIN)
$(LIB_NAME):	ringbuffer.c
	$(CC) -shared -fPIC -Wall $^ -o $@
$(TEST_BIN):	test.c
	$(CC) -std=c99 $^ -o $@ -L. -lrbuf -Wl,-rpath=. -g
clean:
	rm -rf $(LIB_NAME) $(TEST_BIN)

.PHONY:	all test clean
