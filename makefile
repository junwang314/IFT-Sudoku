NT=$(findstring NT,$(OS))
EXE=$(NT:NT=.exe)

all: sudoku sudokuCGI

debug: CFLAGS += -DDEBUG=1

debug: all

sudoku: LDLIBS += -lcurses

sudoku: sudoku.o puzzle.o cursesGui.o twiddleBits.o

sudokuCGI: sudokuCGI.o puzzle.o cgi.o twiddleBits.o

sudoku.o: puzzle.h cursesGui.h

sudokuCGI.o: puzzle.h cgi.h

cursesGui.o: cursesGui.h twiddleBits.h

cgi.o: cgi.h

puzzle.o: puzzle.h twiddleBits.h

twiddleBits.o: twiddleBits.h

src: sudoku.c sudokuCGI.c cursesGui.c cursesGui.h cgi.c cgi.h puzzle.c \
	puzzle.h twiddleBits.c twiddleBits.h makefile

clobber: clean
	rm -f sudoku$(EXE)
	rm -f sudokuCGI$(EXE)

clean:
#	rcsclean sudoku.c
#	rcsclean sudokuCGI.c
#	rcsclean cursesGui.c
#	rcsclean cursesGui.h
#	rcsclean cgi.c
#	rcsclean cgi.h
#	rcsclean puzzle.c
#	rcsclean puzzle.h
#	rcsclean twiddleBits.c
#	rcsclean twiddleBits.h
	rm -f sudoku.o
	rm -f sudokuCGI.o
	rm -f cursesGui.o
	rm -f cgi.o
	rm -f puzzle.o
	rm -f twiddleBits.o
	rm -f sudoku
	rm -f sudokuCGI
#	rcsclean makefile
