CXXFLAGS=-W -Wall -Werror -pedantic -std=c++11
OBJECTS=strset.o strsetconst.o

ifeq ($(debuglevel), 1)
	CXXFLAGS+=-g -D DEBUG
else
	CXXFLAGS+=-O2
endif

all: $(OBJECTS)

clean:
	rm -f $(OBJECTS)

.PHONY: all clean
