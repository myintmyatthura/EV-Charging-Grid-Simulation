CC = mpicc
CFLAGS =
LDFLAGS = -lm
EXECUTABLE = Output
NP = 101

.PHONY: all run clean

all: $(EXECUTABLE) run

$(EXECUTABLE): main.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

run: $(EXECUTABLE)
	@echo "Grid Size: [ 10 x 10 ]"
	@echo "Processor count: 101"
	@echo "Percentage threshold: 20%"
	@echo "Number of Ports: 20"
	@echo "Sleep: 0s"
	mpirun -oversubscribe -np $(NP) ./$(EXECUTABLE)

clean:
	rm -f $(EXECUTABLE)
