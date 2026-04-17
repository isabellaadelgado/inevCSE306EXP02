CC      = gcc
CFLAGS  = -Wall -Wextra -Werror -std=c11 -g -I./source
LDFLAGS = -lssl -lcrypto

CFLAGS  += -I/mingw64/include
LDFLAGS += -L/mingw64/lib

COVFLAGS = --coverage

TARGETS = encoder decoder

SRCS_ENCODER = source/encoder.c source/suffix_tree.c
OBJS_ENCODER = $(SRCS_ENCODER:.c=.o)

SRCS_DECODER = source/decoder.c
OBJS_DECODER = $(SRCS_DECODER:.c=.o)

SRC_CLI   = src/cli/cli.c
OBJ_CLI   = $(SRC_CLI:.c=.o)

SRC_IMAGE = src/image/image.c
OBJ_IMAGE = $(SRC_IMAGE:.c=.o)

TEST_CLI   = tests/testCli
TEST_IMAGE = tests/testImage

TEST_ENCODER = tests/testEncoder
TEST_DECODER = tests/testDecoder
TEST_BASIC   = tests/testBasic

all: $(TARGETS)

encoder: $(OBJS_ENCODER)
	@echo "Linking encoder..."
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

decoder: $(OBJS_DECODER)
	@echo "Linking decoder..."
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

source/%.o: source/%.c
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@ -MMD

src/cli/%.o: src/cli/%.c
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -I./src/cli -c $< -o $@ -MMD

src/image/%.o: src/image/%.c
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -I./src/image -c $< -o $@ -MMD

$(TEST_CLI): tests/testCli.c $(OBJ_CLI)
	@echo "Building testCli..."
	$(CC) $(CFLAGS) -I./src/cli -o $@ $^

$(TEST_IMAGE): tests/testImage.c $(OBJ_IMAGE)
	@echo "Building testImage..."
	$(CC) $(CFLAGS) -I./src/image -o $@ $^

$(TEST_ENCODER): tests/testEncoder.c
	@echo "Building testEncoder..."
	$(CC) $(CFLAGS) -o $@ $<

$(TEST_DECODER): tests/testDecoder.c
	@echo "Building testDecoder..."
	$(CC) $(CFLAGS) -o $@ $<

$(TEST_BASIC): tests/testBasic.c
	@echo "Building testBasic..."
	$(CC) $(CFLAGS) -o $@ $<

test: $(TEST_ENCODER) $(TEST_DECODER) $(TEST_BASIC)
	@echo ""
	@echo "=============================="
	@echo "Running encoder tests..."
	@echo "=============================="
	@./$(TEST_ENCODER)
	@echo ""
	@echo "=============================="
	@echo "Running decoder tests..."
	@echo "=============================="
	@./$(TEST_DECODER)
	@echo ""
	@echo "=============================="
	@echo "Running basic tests..."
	@echo "=============================="
	@./$(TEST_BASIC)
	@echo ""
	@echo "All test suites passed."

tests: test

coverage: clean
	@echo "Building with coverage flags..."
	$(MAKE) CFLAGS="$(CFLAGS) $(COVFLAGS)" LDFLAGS="$(LDFLAGS) $(COVFLAGS)" all test
	@echo ""
	@echo "Generating gcov reports..."
	gcov source/encoder.c source/decoder.c source/suffix_tree.c > coverage_report.txt || true
	@echo ""
	@echo "Coverage summary saved to coverage_report.txt"

clean:
	@echo "Cleaning generated files..."
	rm -f $(TARGETS) $(OBJS_ENCODER) $(OBJS_DECODER) \
	      source/encoder.d source/suffix_tree.d source/decoder.d \
	      $(TEST_ENCODER) $(TEST_DECODER) $(TEST_BASIC) \
	      *.gcov *.gcda *.gcno source/*.gcov source/*.gcda source/*.gcno \
	      tests/*.gcov tests/*.gcda tests/*.gcno coverage_report.txt

.PHONY: all clean test tests coverage
-include source/encoder.d source/suffix_tree.d source/decoder.d
