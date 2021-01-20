
.POSIX:

CC = gcc
CFLAGS = -std=c11 -g
LDFLAGS =
MKDIR = mkdir -p

.PHONY: dirs

all: dirs bin/neuralnet bin/test_neuralnet

dirs: bin bin/obj bin/test_obj
bin:
	$(MKDIR) bin
bin/obj:
	$(MKDIR) bin/obj
bin/test_obj:
	$(MKDIR) bin/test_obj

bin/neuralnet: bin/obj/main.o bin/obj/neuron.o
	$(CC) $(CFLAGS) -o bin/neuralnet bin/obj/main.o bin/obj/neuron.o $(LDFLAGS)

bin/obj/main.o: src/main.c
	$(CC) $(CFLAGS) -o bin/obj/main.o -c src/main.c $(LDFLAGS)

bin/obj/neuron.o: src/neuron.c src/headers/neuron.h
	$(CC) $(CFLAGS) -o bin/obj/neuron.o -c src/neuron.c $(LDFLAGS)

bin/test_neuralnet: bin/test_obj/test.o bin/test_obj/test_utils.o
	$(CC) $(CFLAGS) -o bin/test_neuralnet bin/test_obj/test.o bin/test_obj/test_utils.o $(LDFLAGS)

bin/test_obj/test.o: src/tests/test.c src/tests/headers/test_utils.h
	$(CC) $(CFLAGS) -o bin/test_obj/test.o -c src/tests/test.c -DRUN_TESTS $(LDFLAGS)

bin/test_obj/test_utils.o: src/tests/headers/test_utils.h
	$(CC) $(CFLAGS) -o bin/test_obj/test_utils.o -c src/tests/test_utils.c -DRUN_TESTS $(LDFLAGS)

clean:
	rm -d -r bin
