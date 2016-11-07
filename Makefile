all: bedstead.otf bedstead-ext.otf sample.png title.png extended.png \
     bedstead-10-df.png bedstead-20-df.png

bedstead.sfd: bedstead
	./bedstead > bedstead.sfd

bedstead-ext.sfd: bedstead
	./bedstead --extended > bedstead-ext.sfd

%.otf %-10.bdf %-20.bdf: %.sfd
	fontforge -lang=ff \
	    -c 'Open($$1); BitmapsAvail([10, 20]); Generate($$2, "bdf")' $< $@

%.pfa %.afm: %.sfd
	fontforge -lang=ff -c 'Open($$1); Generate($$2)' $< $@

%.png: %.ps bedstead.pfa bedstead-ext.pfa
	gs -q -dSAFER -sDEVICE=pnggray -dTextAlphaBits=4 -o $@ \
		bedstead.pfa bedstead-ext.pfa $<

bedstead-%-df.png: df.ps bedstead.pfa
	gs -q -dSAFER -dsize=$* -sDEVICE=png16m -o $@ bedstead.pfa $<

.PHONY: clean
clean:
	rm -f bedstead *.sfd *.otf *.bdf *.pfa *.png

DISTFILES = bedstead.c Makefile COPYING \
	bedstead.sfd bedstead.otf bedstead.pfa bedstead.afm \
	bedstead-ext.sfd bedstead-ext.otf bedstead-ext.pfa bedstead-ext.afm \
	bedstead-10.bdf bedstead-20.bdf \
	bedstead-10-df.png bedstead-20-df.png

.PHONY: dist

dist: $(DISTFILES) bedstead
	rm -rf bedstead-$$(sed -n 's/^Version: //p' < bedstead.sfd)
	mkdir bedstead-$$(sed -n 's/^Version: //p' < bedstead.sfd)
	ln $(DISTFILES) \
	    bedstead-$$(sed -n 's/^Version: //p' < bedstead.sfd)
	zip -r bedstead-$$(sed -n 's/^Version: //p' < bedstead.sfd).zip \
	    bedstead-$$(sed -n 's/^Version: //p' < bedstead.sfd)
	rm -r bedstead-$$(sed -n 's/^Version: //p' < bedstead.sfd)
