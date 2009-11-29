all: bedstead.otf

bedstead.sfd: bedstead
	./bedstead > bedstead.sfd

bedstead.otf: bedstead.sfd
	fontforge -lang=ff -c 'Open($$1); Generate($$2)' \
		bedstead.sfd bedstead.otf

clean: .PHONY
	rm -f bedstead bedstead.sfd bedstead.otf
