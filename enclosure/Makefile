# a simple makefile to build and view stl for enclosure

SCAD_FILES := $(wildcard *.scad)

run: inc.stl view

inc.stl: $(SCAD_FILES)
	openscad -o inc.stl inc.scad

view:
	rish am force-stop com.performance.meshview
	termux-share -d inc.stl

.PHONY: view run
