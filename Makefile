CC := clang
CFLAGS := -g -Wall -Wno-deprecated-declarations -Werror

all: battleship

clean:
	rm -f battleship

battleship: cell.c board.c board.h battleship.c gameMessage.c gameMessage.h socket.h 
	$(CC) $(CFLAGS) -o battleship board.c cell.c gameMessage.c battleship.c

zip:
	@echo "Generating battleship.zip file to submit to Gradescope..."
	@zip -q -r battleship.zip . -x .git/\* .vscode/\* .clang-format .gitignore battleship
	@echo "Done. Please upload battleship.zip to Gradescope."

format:
	@echo "Reformatting source code."
	@clang-format -i --style=file $(wildcard *.c) $(wildcard *.h)
	@echo "Done."

.PHONY: all clean zip format