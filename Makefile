all: doc

doc: man/schooner-blend.1.html man/schooner-cloud.1.html man/schooner-contrast.1.html man/schooner-multibalance.1.html man/schooner-stitch.1.html

man/schooner-blend.1.html: man/schooner-blend.1.ronn
man/schooner-cloud.1.html: man/schooner-cloud.1.ronn
man/schooner-contrast.1.html: man/schooner-contrast.1.ronn
man/schooner-multibalance.1.html: man/schooner-multibalance.1.ronn
man/schooner-stitch.1.html: man/schooner-stitch.1.ronn

man/%.html: man/%.ronn
	ronn --manual=schooner-tk --organization=propublica $<

clean:
	rm man/*.html man/*.1

.PHONY: all clean