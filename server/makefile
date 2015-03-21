PRO = simplesrv
SRC = main.cpp parser.cpp
OBJ = ${SRC:.cpp=.o}

INCS =
LIBS = -ljsoncpp
CFLAGS = -std=c99 -Os
CPPFLAGS = -std=c++11 -Os ${INCS} -g
LDFLAGS = ${LIBS}
CC = g++

all: ${PRO}

options:
	@echo simplesrv build options:
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

