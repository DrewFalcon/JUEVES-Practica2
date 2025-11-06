CC = gcc -g
CFLAGS = -Wall -Wextra -pedantic -ansi
LDLIBS = -lodbc -lcurses -lpanel -lmenu -lform

# comands/vaules for the database
export PGDATABASE:=flight
export PGUSER :=alumnodb
export PGPASSWORD :=alumnodb
export PGCLIENTENCODING:=UTF8
export PGHOST:=localhost

DBNAME =$(PGDATABASE)
PSQL = psql
CREATEDB = createdb
DROPDB = dropdb --if-exists
PG_DUMP = pg_dump
PG_RESTORE = pg_restore




# recompile if this header changes
HEADERS = odbc.h bpass.h  lmenu.h search.h windows.h loop.h

EXE = menu
OBJ = $(EXE).o bpass.o   loop.o  search.o windows.o

all : dropdb createdb restore $(EXE)

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

# Comands for the database

createdb:
	@echo Creando BBDD
	@$(CREATEDB)
dropdb:
	@echo Eliminando BBDD
	@$(DROPDB) $(DBNAME)
	rm -f *.log
dump:
	@echo creando dumpfile
	@$(PG_DUMP) > $(DBNAME).sql
restore:
	@echo restore data base
	@cat $(DBNAME).sql | $(PSQL)  


test_search:
	@echo search_sql_test: "please insert here a short description" | tee search_sql_test.log
	@cat search_sql_test.sql | $(PSQL) | tee -a search_sql_test.log
