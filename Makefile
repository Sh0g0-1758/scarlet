.PHONY: build test clean

build:
	@echo "Building scarlet..."
	@mkdir -p build
	@g++-13 -std=c++20 ./driver/driver.cc -o ./build/scarlet -I .
	@echo "Done."

test: build
	@echo "Running lexer test"
	@./test_compiler ./build/scarlet --chapter 1 --stage lex
	@echo "Running parser test"
	@./test_compiler ./build/scarlet --chapter 1 --stage parse
	@echo "Running Codegen test"
	@./test_compiler ./build/scarlet --chapter 1 --stage codegen
	@echo "Running test all-1"
	@./test_compiler ./build/scarlet --chapter 1

clean:
	@echo "Cleaning up..."
	@rm -rf build