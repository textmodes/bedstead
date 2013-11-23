all: bedstead.otf sample.png title.png bedstead-10-df.png bedstead-20-df.png

bedstead.sfd: bedstead
	./bedstead > bedstead.sfd

%.otf: %.sfd
	fontforge -lang=ff -c 'Open($$1); BitmapsAvail([10, 20]); Generate($$2, "bdf")' $< $@

%.pfa: %.sfd
	fontforge -lang=ff -c 'Open($$1); Generate($$2)' $< $@

%.png: %.ps bedstead.pfa
	gs -dSAFER -sDEVICE=pnggray -dTextAlphaBits=4 \
		-o $@ bedstead.pfa $<

bedstead-%-df.png: df.ps bedstead.pfa
	gs -dSAFER -dsize=$* -sDEVICE=png16m \
		-o $@ bedstead.pfa $<

.PHONY: clean
clean:
	rm -f bedstead *.sfd *.otf *.bdf *.pfa *.png
