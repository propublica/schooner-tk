# this makefile is a complete mess but I need help cleaning it up

CC = clang++
CXX = clang++
RONN = $(shell find man -name "*.ronn")
HTML = $(RONN:.ronn=.html)

SRCS = $(shell find src -name "*.cc")
OBJS = $(SRCS:.o=.cc)
BINS = $(basename $(OBJS))

all: doc binaries $(HTML)
doc: $(HTML)
binaries: $(BINS)

man/%.html: man/%.ronn
	ronn --manual=schooner-tk --organization=propublica $<

CXXFLAGS ?= $(shell gdal-config --cflags) -g -std=c++11 -stdlib=libc++ $(shell pkg-config --cflags opencv) -I./src/
LDLIBS ?= $(shell gdal-config --libs) $(shell pkg-config --libs opencv)

bin:
	mkdir -p bin

src/schooner-blend.o: src/schooner-blend.cc src/utils.h bin
src/schooner-cloud.o: src/schooner-cloud.cc src/utils.h bin
src/schooner-contrast.o: src/schooner-contrast.cc src/utils.h bin
src/schooner-multibalance.o: src/schooner-multibalance.cc src/utils.h bin
src/schooner-stitch.o: src/schooner-stitch.cc src/utils.h bin
src/schooner-ndvi.o: src/schooner-ndvi.cc src/utils.h bin

clean:
	rm man/*.html man/*.1
	rm src/*.o
	rm -r bin

.PHONY: all clean doc binaries
