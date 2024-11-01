CFLAGS = -std=c99 -pedantic -Werror -Wall -Wextra -Wvla -Isrc/
LDFLAGS =
CC = gcc

SRC = src/main.c src/ast_eval.c src/lexer.c src/parser.c src/queue.c src/stack.c
OBJ = $(SRC:.c=.o)
BIN = myfind

all: myfind

myfind: $(OBJ)
	$(CC) -o $(BIN) $^

debug: CFLAGS += -fsanitize=address -g
debug: $(OBJ)
	$(CC) $(CFLAGS) $^ -o $(BIN)

check: debug
	bash tests/test.sh $(BIN)

coverage: LDFLAGS += -lgcov --coverage
coverage: CFLAGS += -fPIC --coverage -g
coverage: debug check
	gcovr --html --html-nested --output=report.html

clean:
	find . -type f \( -name "*.gc*" -o -name "*.css" -o -name "*.html" \) -delete
	$(RM) $(BIN) $(TESTS_BIN)
	$(RM) $(OBJ) $(TESTS_OBJ)
