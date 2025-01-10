.PHONY: build test clean

CXX = g++-13
CXXFLAGS = -std=c++20 -I .

BUILD_DIR = build

SRCS = driver/driver.cc \
       src/ast/ast.cc \
       src/lexer/lexer.cc \
       src/parser/parser.cc \
       src/regex/regex.cc \
       src/tokens/tokens.cc

TARGET = $(BUILD_DIR)/scarlet

build:
	@echo "Building scarlet..."
	@mkdir -p $(BUILD_DIR)
	@$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)
	@echo "Done."

test: build
	@echo "Running lexer-1 test"
	@./test_compiler ./build/scarlet --chapter 1 --stage lex
	@echo "Running parser-1 test"
	@./test_compiler ./build/scarlet --chapter 1 --stage parse
	@echo "Running Codegen-1 test"
	@./test_compiler ./build/scarlet --chapter 1 --stage codegen
	@echo "Running test all-1"
	@./test_compiler ./build/scarlet --chapter 1
	@echo "Running lexer-2 test"
	@./test_compiler ./build/scarlet --chapter 2 --stage lex
	@echo "Running parser-2 test"
	@./test_compiler ./build/scarlet --chapter 2 --stage parse
	@echo "Running scar-2 test"
	@./test_compiler ./build/scarlet --chapter 2 --stage tacky
	@echo "Running Codegen-2 test"
	@./test_compiler ./build/scarlet --chapter 2 --stage codegen
	@echo "Running test all-2"
	@./test_compiler ./build/scarlet --chapter 2
clean:
	@echo "Cleaning up..."
	@rm -rf build