PRO = test
SRC = basic.cpp
OBJ = ${SRC:.cpp=.o}

INCS = 
LIBS = -lboost_system -lboost_filesystem -lboost_unit_test_framework 
CFLAGS = -std=c99 -Os
CPPFLAGS = -std=c++11 -Os ${INCS} -g -Wunused
LDFLAGS = ${LIBS}
CC = g++

all: ${PRO}

options:
	@echo test build options:
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
	@rm -f database.db

.PHONY: all clean

