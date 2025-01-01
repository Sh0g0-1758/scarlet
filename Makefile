.PHONY: build test clean

build:
	@echo "Building scarlet..."
	@mkdir -p build
	@g++-13 -std=c++20 ./driver/driver.cc -o ./build/scarlet -I .
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
clean:
	@echo "Cleaning up..."
	@rm -rf build