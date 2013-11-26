all: bedstead.otf sample.png title.png bedstead-10-df.png bedstead-20-df.png

bedstead.sfd: bedstead
	./bedstead > bedstead.sfd

%.otf %-10.bdf %-20.bdf: %.sfd
	fontforge -lang=ff \
	    -c 'Open($$1); BitmapsAvail([10, 20]); Generate($$2, "bdf")' $< $@

%.pfa %.afm: %.sfd
	fontforge -lang=ff -c 'Open($$1); Generate($$2)' $< $@

%.png: %.ps bedstead.pfa
	gs -q -dSAFER -sDEVICE=pnggray -dTextAlphaBits=4 -o $@ bedstead.pfa $<

bedstead-%-df.png: df.ps bedstead.pfa
	gs -q -dSAFER -dsize=$* -sDEVICE=png16m -o $@ bedstead.pfa $<

.PHONY: clean
clean:
	rm -f bedstead *.sfd *.otf *.bdf *.pfa *.png

DISTFILES = bedstead.c Makefile \
	bedstead.sfd bedstead.otf bedstead.pfa bedstead.afm \
	bedstead-10.bdf bedstead-20.bdf \
	bedstead-10-df.png bedstead-20-df.png

.PHONY: dist

dist: $(DISTFILES)
	mkdir bedstead-dist
	ln $(DISTFILES) bedstead-dist
	zip -r bedstead.zip bedstead-dist
	rm -r bedstead-dist
