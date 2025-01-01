# a simple Makefile

SENDER = termux-share -a send
VIEWER = viewpdf

CHAPTERS := $(wildcard chapter_*/chapter.tex)
WORKS := $(wildcard work_*/work.tex)

SECTIONS := $(wildcard chapter_*/section_*/section.tex)
SECTIONS := $(wildcard work_*/section_*/section.tex)

TIKZPICS := $(wildcard */tikzpics/*.tex)

ABSTRACT := abstract_template.tex abstract.tex

build: main.pdf view

force:
	lunatikz build main.tex
	pdflatex -shell-escape -halt-on-error main.tex
	$(VIEWER) main.pdf

main.pdf: main.tex preamble.tex colorscheme.sty $(ABSTRACT) $(CHAPTERS) $(SECTIONS) $(WORKS) $(TIKZPICS)
	lunatikz build main.tex
	pdflatex -shell-escape -halt-on-error main.tex

abstract: abstract.pdf view_abstract

abstract.pdf: preamble.tex colorscheme.sty $(ABSTRACT)
	pdflatex -shell-escape -halt-on-error abstract.tex

view_abstract:
	$(VIEWER) abstract.pdf

view:
	$(VIEWER) main.pdf

send:
	$(SENDER) main.pdf

.PHONY: view build force view_abstract abstract
