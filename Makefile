all: doc binaries

doc: man/schooner-blend.1.html man/schooner-cloud.1.html man/schooner-contrast.1.html man/schooner-multibalance.1.html man/schooner-stitch.1.html

man/schooner-blend.1.html: man/schooner-blend.1.ronn
man/schooner-cloud.1.html: man/schooner-cloud.1.ronn
man/schooner-contrast.1.html: man/schooner-contrast.1.ronn
man/schooner-multibalance.1.html: man/schooner-multibalance.1.ronn
man/schooner-stitch.1.html: man/schooner-stitch.1.ronn

man/%.html: man/%.ronn
	ronn --manual=schooner-tk --organization=propublica $<

CXXFLAGS ?= $(shell gdal-config --cflags) -g -std=c++11 -stdlib=libc++ $(shell pkg-config --cflags opencv)
LDLIBS ?= $(shell gdal-config --libs) $(shell pkg-config --libs opencv)

bin:
	mkdir -p bin

src/schooner-blend.o: src/schooner-blend.cc src/utils.h
src/schooner-cloud.o: src/schooner-cloud.cc src/utils.h
src/schooner-contrast.o: src/schooner-contrast.cc src/utils.h
src/schooner-multibalance.o: src/schooner-multibalance.cc src/utils.h
src/schooner-stitch.o: src/schooner-stitch.cc src/utils.h

bin/schooner-blend: src/schooner-blend.o bin
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS) $<

bin/schooner-cloud: src/schooner-cloud.o bin
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS) $<

bin/schooner-contrast: src/schooner-contrast.o bin
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS) $<

bin/schooner-multibalance: src/schooner-multibalance.o bin
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS) $<

bin/schooner-stitch: src/schooner-stitch.o bin
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS) $<

binaries: bin/schooner-blend bin/schooner-cloud bin/schooner-contrast bin/schooner-multibalance bin/schooner-stitch

clean:
	rm man/*.html man/*.1
	rm bin/*
	rm -r bin

.PHONY: all clean doc