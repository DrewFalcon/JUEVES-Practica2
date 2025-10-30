CC = gcc -g
CFLAGS = -Wall -Wextra -pedantic -ansi
LDLIBS = -lodbc -lcurses -lpanel -lmenu -lform
PSQL = psql

# recompile if this header changes
HEADERS = odbc.h bpass.h  lmenu.h search.h windows.h loop.h

EXE = menu
OBJ = $(EXE).o bpass.o   loop.o  search.o windows.o

all : $(EXE)

# compile all files ending in *.c
%.o: %.c $(HEADERS)
	@echo Compiling $<...
	$(CC) $(CFLAGS) -c -o $@ $<

# link binary
$(EXE): $(DEPS) $(OBJ)
	$(CC) -o $(EXE) $(OBJ) $(LDLIBS)

clean:
	rm -f *.o core $(EXE)

run:
	./$(EXE)

valgrind:
	valgrind --leak-check=full ./$(EXE)

test_search:
	@echo search_sql_test: "please insert here a short description" | tee search_sql_test.log
	@cat search_sql_test.sql | $(PSQL) | tee -a search_sql_test.log
