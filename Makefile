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

CXXFLAGS = -g $(FLAGS) -I./src/ $(shell gdal-config --cflags) $(shell gdal-config --libs) $(shell pkg-config --cflags --libs opencv)

clean:
	rm man/*.html man/*.1 $(BINS)

install: all
	install $(BINS) /usr/local/bin
	install man/*.1 /usr/local/share/man/man1/

.PHONY: all clean doc binaries install
