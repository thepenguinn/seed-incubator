# a simple Makefile

SENDER = termux-share -a send
VIEWER = viewpdf

CHAPTERS := $(wildcard chapter_*/chapter.tex)
WORKS := $(wildcard work_*/work.tex)

SECTIONS := $(wildcard chapter_*/section_*/section.tex)
SECTIONS := $(wildcard work_*/section_*/section.tex)

TIKZPICS := $(wildcard */tikzpics/*.tex)

ABSTRACT := abstract_template.tex abstract.tex abstract.bib

PRJPLAN := prjplan.tex

build: main.pdf view

force:
	lunatikz build main.tex
	pdflatex -shell-escape -halt-on-error main.tex
	biber main
	pdflatex -shell-escape -halt-on-error main.tex
	$(VIEWER) main.pdf

main.pdf: main.tex preamble.tex colorscheme.sty $(ABSTRACT) $(CHAPTERS) $(SECTIONS) $(WORKS) $(TIKZPICS)
	lunatikz build main.tex
	pdflatex -shell-escape -halt-on-error main.tex
	biber main
	pdflatex -shell-escape -halt-on-error main.tex

abstract: abstract.pdf view_abstract

abstract.pdf: preamble.tex colorscheme.sty $(ABSTRACT)
	pdflatex -shell-escape -halt-on-error abstract.tex
	biber abstract
	pdflatex -shell-escape -halt-on-error abstract.tex

view_abstract:
	$(VIEWER) abstract.pdf

prjplan: prjplan.pdf view_prjplan

prjplan.pdf: preamble.tex colorscheme.sty $(PRJPLAN)
	pdflatex -shell-escape -halt-on-error prjplan.tex

abt_plan: prjplan.pdf abstract.pdf
	pdftk prjplan.pdf abstract.pdf output group_10.pdf
	$(VIEWER) group_10.pdf

view_prjplan:
	$(VIEWER) prjplan.pdf

view:
	$(VIEWER) main.pdf

send:
	$(SENDER) main.pdf

.PHONY: view build force view_abstract abstract prjplan abt_plan
