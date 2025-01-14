# a simple Makefile

SENDER = termux-share -a send
VIEWER = viewpdf

SECTIONS := $(wildcard section_*/section.tex)

TIKZPICS := $(shell find -type f -wholename "*/tikzpics/*.tex")

build: chapter.pdf view

force:
	lunatikz build chapter.tex
	pdflatex -shell-escape -halt-on-error chapter.tex
	$(VIEWER) chapter.pdf

chapter.pdf: chapter.tex chapter_header.tex ../preamble.tex ../colorscheme.sty ../main.tex $(SECTIONS) $(TIKZPICS)
	lunatikz build chapter.tex
	pdflatex -shell-escape -halt-on-error chapter.tex

view:
	$(VIEWER) chapter.pdf

send:
	$(SENDER) chapter.pdf

icon: _icon_lunatikz icon/icon.png

_icon_lunatikz:
	lunatikz build --style icon icon/icon.tex

icon/icon.pdf: section_04/tikzpics/icon/endSapIcon.pdf
	echo cp...
	cp section_04/tikzpics/icon/endSapIcon.pdf icon/icon.pdf

icon/icon.svg: icon/icon.pdf
	pdf2svg icon/icon.pdf icon/icon.svg

icon/icon.png: icon/icon.svg
	rsvg-convert -w 1920 -h 1920 -o icon/icon.png icon/icon.svg

.PHONY: view build force icon _icon_lunatikz _gen_icons
