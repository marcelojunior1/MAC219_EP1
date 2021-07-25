OUTPUT=mandelbrot_ompi
MCC=mpicc

.PHONY: all

all: $(OUTPUT)

$(OUTPUT): $(OUTPUT).c clean
	$(MCC) -o $(OUTPUT) $(OUTPUT).c

.PHONY: clean
clean:
	rm -f $(OUTPUT) output.ppm


