OUTPUT2=mandelbrot_ompi
OUTPUT=mandelbrot
MCC=mpicc
CC=gcc
CC_OPT=-std=c11 -D_POSIX_C_SOURCE=199309L
CC_OMP=-fopenmp
CC_PTH=-pthread

.PHONY: all

all: $(OUTPUT2) $(OUTPUT2)_pth $(OUTPUT2)_omp $(OUTPUT)_omp $(OUTPUT)_pth $(OUTPUT)_seq

$(OUTPUT2): $(OUTPUT2).c clean
	$(MCC) -o $(OUTPUT2) $(OUTPUT2).c

$(OUTPUT2)_pth: $(OUTPUT2)_pth.c
	$(MCC) -o $(OUTPUT2)_pth  $(CC_PTH) $(OUTPUT2)_pth.c

$(OUTPUT2)_omp: $(OUTPUT2)_omp.c
	$(MCC) -o $(OUTPUT2)_omp  $(CC_OMP) $(OUTPUT2)_omp.c

$(OUTPUT)_omp: $(OUTPUT)_omp.c
	$(CC) -o $(OUTPUT)_omp $(CC_OPT) $(CC_OMP) $(OUTPUT)_omp.c

$(OUTPUT)_pth: $(OUTPUT)_pth.c
	$(CC) -o $(OUTPUT)_pth $(CC_OPT) $(CC_PTH) $(OUTPUT)_pth.c

$(OUTPUT)_seq: $(OUTPUT)_seq.c
	$(CC) -o $(OUTPUT)_seq $(CC_OPT) $(OUTPUT)_seq.c

.PHONY: clean
clean:
	rm -f $(OUTPUT2) $(OUTPUT2)_pth $(OUTPUT2)_omp $(OUTPUT)_omp $(OUTPUT)_pth $(OUTPUT)_seq output.ppm