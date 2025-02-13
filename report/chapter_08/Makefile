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

.PHONY: view build force
