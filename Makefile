all: bedstead.otf sample.png title.png

bedstead.sfd: bedstead
	./bedstead > bedstead.sfd

.SUFFIXES: .ps .png .sfd .otf .pfa

.sfd.otf:
	fontforge -lang=ff -c 'Open($$1); BitmapsAvail([10, 20]); Generate($$2, "bdf")' $< $@

.sfd.pfa:
	fontforge -lang=ff -c 'Open($$1); Generate($$2)' $< $@

.ps.png: bedstead.pfa
	gs -dSAFER -sDEVICE=pnggray -dTextAlphaBits=4 \
		-o $@ bedstead.pfa $<

df.png: df.ps bedstead.pfa
	gs -dSAFER -sDEVICE=png16m \
		-o $@ bedstead.pfa $<

.PHONY: clean
clean:
	rm -f bedstead *.sfd *.otf *.bdf *.pfa *.png
