all: bedstead.otf sample.png title.png

bedstead.sfd: bedstead
	./bedstead > bedstead.sfd

.SUFFIXES: .ps .png .sfd .otf .pfa

.sfd.otf:
	fontforge -lang=ff -c 'Open($$1); BitmapsAvail([10, 20]); Generate($$2, "bdf")' $< $@

.sfd.pfa:
	fontforge -lang=ff -c 'Open($$1); Generate($$2)' $< $@

.ps.png: bedstead.pfa
	gs -dSAFER -dBATCH -dNOPAUSE -sDEVICE=pnggray -dTextAlphaBits=4 \
		-sOutputFile=$@ bedstead.pfa $<

df.png: df.ps bedstead.pfa
	gs -dSAFER -dBATCH -dNOPAUSE -sDEVICE=png16m \
		-sOutputFile=$@ bedstead.pfa $<

clean: .PHONY
	rm -f bedstead *.sfd *.otf *.bdf *.pfa *.png
