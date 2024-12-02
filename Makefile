build:
	@echo "Building scarlet..."
	@mkdir build
	@g++-13 -std=c++20 ./driver/driver.cc -o ./build/scarlet -I .
	@echo "Done."

test:
	@echo "Running lexer test"
	@./test_compiler ./build/scarlet --chapter 1 --stage lex
	@echo "Running parser test"
	@./test_compiler ./build/scarlet --chapter 1 --stage parse
	@echo "Done."

clean:
	@echo "Cleaning up..."
	@rm -rf build