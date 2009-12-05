all: bedstead.otf sample.png title.png

bedstead.sfd: bedstead
	./bedstead > bedstead.sfd

.SUFFIXES: .ps .png .sfd .otf .pfa

.sfd.otf:
	fontforge -lang=ff -c 'Open($$1); Generate($$2)' ${.IMPSRC} ${.TARGET}

.sfd.pfa:
	fontforge -lang=ff -c 'Open($$1); Generate($$2)' ${.IMPSRC} ${.TARGET}

.ps.png: bedstead.pfa
	gs -dSAFER -dBATCH -dNOPAUSE -sDEVICE=pnggray \
		-sOutputFile=${.TARGET} bedstead.pfa ${.IMPSRC}

clean: .PHONY
	rm -f bedstead *.sfd *.otf *.pfa *.png
