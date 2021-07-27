OUTPUT=mandelbrot_ompi
MCC=mpicc
CC=gcc
CC_OPT=-std=c11 -D_POSIX_C_SOURCE=199309L
CC_OMP=-fopenmp
CC_PTH=-lpthread

.PHONY: all

all: $(OUTPUT) $(OUTPUT)_pth

$(OUTPUT): $(OUTPUT).c clean
	$(MCC) -o $(OUTPUT) $(OUTPUT).c

$(OUTPUT)_pth: $(OUTPUT)_pth.c
	$(MCC) -o $(OUTPUT)_pth  $(CC_PTH) $(OUTPUT)_pth.c

.PHONY: clean
clean:
	rm -f $(OUTPUT) $(OUTPUT)_pth output.ppm