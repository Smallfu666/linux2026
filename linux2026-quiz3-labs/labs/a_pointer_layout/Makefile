LAB_DIR := $(dir $(lastword $(MAKEFILE_LIST)))

.PHONY: all matrix demo clean

all: matrix demo

matrix:
	./run.sh matrix

demo:
	./run.sh demo

clean:
	rm -rf build artifacts compile_matrix.md
