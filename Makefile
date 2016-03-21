OS := $(shell uname -s)
ifeq ($(OS),Linux)
	CC = g++
	CXX = g++
	FLAGS = -std=c++11
endif

ifeq ($(OS),Darwin)
	CC = clang++
	CXX = clang++
	FLAGS = -std=c++11 -stdlib=libc++
endif

RONN = $(wildcard man/*.ronn)
HTML = $(RONN:.ronn=.html)

SRCS = $(wildcard src/*.cc)
BINS = $(basename $(SRCS))

all: doc binaries $(HTML)
doc: $(HTML)
binaries: $(BINS)

man/%.html: man/%.ronn
	ronn --manual=schooner-tk --organization=propublica $<

CXXFLAGS = $(shell gdal-config --cflags) -g $(FLAGS) $(shell pkg-config --cflags opencv) -I./src/
LDLIBS = $(shell gdal-config --libs) $(filter-out -lippicv, $(shell pkg-config --libs opencv)) $(FLAGS)

clean:
	rm man/*.html man/*.1 $(BINS)

install: all
	install $(BINS) /usr/local/bin
	install man/*.1 /usr/local/share/man/man1/

.PHONY: all clean doc binaries install
