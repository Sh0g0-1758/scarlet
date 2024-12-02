build:
	@echo "Building scarlet..."
	@mkdir build
	@g++ -std=c++20 ./driver/driver.cc -o ./build/scarlet -I .
	@echo "Done."

test:
	@echo "Running lexer test"
	@./test_compiler ./build/scarlet --chapter 1 --stage lex
	@echo "Done."