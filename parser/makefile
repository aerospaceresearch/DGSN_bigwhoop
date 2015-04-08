PRO = parser
SRC = main.cpp parser.cpp log.cpp options.cpp
OBJ = ${SRC:.cpp=.o}

INCS = -I/usr/local/include/soci
LIBS = -ljsoncpp -lsoci_core -lsoci_sqlite3 -Wl,-rpath=/usr/local/lib64/\
			 -lboost_program_options
CFLAGS = -std=c99 -Os
CPPFLAGS = -std=c++11 -Os ${INCS} -g -Wunused
LDFLAGS = ${LIBS}
CC = g++

all: ${PRO}

options:
	@echo parser build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "CPPFLAGS = ${CPPFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

${PRO}: ${OBJ}
	@echo CC -o $@
	@${CC} -o $@ ${OBJ} ${LDFLAGS}
	@strip ${PRO}

.cpp.o:
	@echo CC $<
	@${CC} -c ${CPPFLAGS} $<

clean:
	@echo cleaning
	@rm -f ${PRO} ${OBJ}

.PHONY: all clean

