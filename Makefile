CC = clang++
CXX = clang++
RONN = $(wildcard man/*.ronn)
HTML = $(RONN:.ronn=.html)

SRCS = $(wildcard src/*.cc)
BINS = $(basename $(SRCS))

all: doc binaries $(HTML)
doc: $(HTML)
binaries: $(BINS)

man/%.html: man/%.ronn
	ronn --manual=schooner-tk --organization=propublica $<

CXXFLAGS = $(shell gdal-config --cflags) -std=c++11 -stdlib=libc++ $(shell pkg-config --cflags opencv) -I./src/ -MMD -MP
LDLIBS = $(shell gdal-config --libs) $(shell pkg-config --libs opencv)

clean: all
	rm man/*.html man/*.1 $(BINS)

install: all
	install $(BINS) /usr/local/bin
	install man/*.1 /usr/local/share/man/man1/

.PHONY: all clean doc binaries install
