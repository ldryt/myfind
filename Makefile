CFLAGS = -std=c99 -pedantic -Werror -Wall -Wextra -Wvla -Isrc/
LDFLAGS =
CC = gcc

SRC = src/main.c src/ast.c src/lexer.c src/parser.c src/queue.c src/stack.c
OBJ = $(SRC:.c=.o)
BIN = myfind

TESTS_SRC = 
TESTS_OBJ = $(TESTS_SRC:.c=.o)
TESTS_BIN = myfind_test

all: myfind

myfind: $(OBJ)
	$(CC) -o $(BIN) $^

test: LDFLAGS += -lcriterion
test: CFLAGS += -g
test: $(OBJ) $(TESTS_OBJ)
	$(CC) $(LDFLAGS) -o $(TESTS_BIN) $^

debug: CFLAGS += -fsanitize=address -g
debug: $(OBJ)
	$(CC) $(CFLAGS) $^ -o $(BIN)

check:
	sh tests/functional_tests.sh

coverage: LDFLAGS += -lgcov --coverage
coverage: CFLAGS += -fPIC --coverage -g
coverage: debug check
	gcovr --html --html-nested --output=report.html

clean:
	find . -type f \( -name "*.gc*" -o -name "*.css" -o -name "*.html" \) -delete
	$(RM) $(BIN) $(TESTS_BIN)
	$(RM) $(OBJ) $(TESTS_OBJ)
