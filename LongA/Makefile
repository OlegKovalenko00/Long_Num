PRECISION ?= 100

default_target:
	cmake -S . -B build && cd build && make

pi:
	cd build && ./pi $(PRECISION)

test:
	cd build && ./test

clean:
	rm -rf build

.PHONY: default_target pi test clean