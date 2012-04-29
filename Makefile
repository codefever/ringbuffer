LIB_NAME:=librbuf.so
TEST_BIN:=rbtest
CC:=clang

all:	$(LIB_NAME) test
test:	$(TEST_BIN)
%.o:	%.c
	$(CC) -fPIC -c $<
$(LIB_NAME):	ringbuffer.o
	$(CC) -shared -fPIC -Wall $^ -o $@
$(TEST_BIN):	test.o
	$(CC) $^ -o $@ -L. -lrbuf -Wl,-rpath=. -g
clean:
	rm -rf $(LIB_NAME) $(TEST_BIN) *.o

.PHONY:	all test clean
