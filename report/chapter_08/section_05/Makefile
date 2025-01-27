# a simple Makefile

SENDER = termux-share -a send
VIEWER = viewpdf

TIKZPICS := $(shell find -type f -wholename "*/tikzpics/*.tex")

build: section.pdf view

force:
	lunatikz build section.tex
	pdflatex -shell-escape -halt-on-error section.tex
	$(VIEWER) section.pdf

section.pdf: section.tex section_header.tex ../../preamble.tex ../../colorscheme.sty ../../main.tex $(TIKZPICS)
	lunatikz build section.tex
	pdflatex -shell-escape -halt-on-error section.tex

view:
	$(VIEWER) section.pdf

send:
	$(SENDER) section.pdf

.PHONY: view build force
