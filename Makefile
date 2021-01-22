
.POSIX:

CC = gcc
CFLAGS = -std=c11 -g
LDFLAGS = -lm
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

bin/neuralnet: bin/obj/main.o bin/obj/neuron.o bin/obj/file_array.o bin/obj/adj_matrix.o bin/obj/math_utils.o
	$(CC) $(CFLAGS) -o bin/neuralnet bin/obj/main.o bin/obj/neuron.o bin/obj/file_array.o bin/obj/adj_matrix.o bin/obj/math_utils.o $(LDFLAGS)

bin/obj/main.o: src/main.c
	$(CC) $(CFLAGS) -o bin/obj/main.o -c src/main.c $(LDFLAGS)

bin/obj/neuron.o: src/neuron.c src/headers/neuron.h
	$(CC) $(CFLAGS) -o bin/obj/neuron.o -c src/neuron.c $(LDFLAGS)

bin/obj/file_array.o: src/file_array.c src/headers/file_array.h
	$(CC) $(CFLAGS) -o bin/obj/file_array.o -c src/file_array.c $(LDFLAGS)

bin/obj/adj_matrix.o: src/adj_matrix.c src/headers/adj_matrix.h
	$(CC) $(CFLAGS) -o bin/obj/adj_matrix.o -c src/adj_matrix.c $(LDFLAGS)

bin/obj/math_utils.o: src/math_utils.c src/headers/math_utils.h
	$(CC) $(CFLAGS) -o bin/obj/math_utils.o -c src/math_utils.c $(LDFLAGS)

bin/test_neuralnet: bin/test_obj/test.o bin/test_obj/test_utils.o bin/test_obj/test_file_array.o bin/test_obj/test_adj_matrix.o bin/test_obj/test_math_utils.o bin/test_obj/neuron.o bin/test_obj/file_array.o bin/test_obj/adj_matrix.o bin/test_obj/math_utils.o
	$(CC) $(CFLAGS) -o bin/test_neuralnet bin/test_obj/test.o bin/test_obj/test_utils.o bin/test_obj/test_file_array.o bin/test_obj/test_adj_matrix.o bin/test_obj/test_math_utils.o bin/test_obj/neuron.o bin/test_obj/file_array.o bin/test_obj/adj_matrix.o bin/test_obj/math_utils.o $(LDFLAGS)

bin/test_obj/test.o: src/tests/test.c src/tests/headers/test_utils.h
	$(CC) $(CFLAGS) -o bin/test_obj/test.o -c src/tests/test.c -DRUN_TESTS $(LDFLAGS)

bin/test_obj/test_utils.o: src/tests/test_utils.c src/tests/headers/test_utils.h
	$(CC) $(CFLAGS) -o bin/test_obj/test_utils.o -c src/tests/test_utils.c -DRUN_TESTS $(LDFLAGS)

bin/test_obj/test_file_array.o: src/tests/test_file_array.c src/tests/headers/test_file_array.h
	$(CC) $(CFLAGS) -o bin/test_obj/test_file_array.o -c src/tests/test_file_array.c -DRUN_TESTS $(LDFLAGS)

bin/test_obj/test_adj_matrix.o: src/tests/test_adj_matrix.c src/tests/headers/test_adj_matrix.h
	$(CC) $(CFLAGS) -o bin/test_obj/test_adj_matrix.o -c src/tests/test_adj_matrix.c -DRUN_TESTS $(LDFLAGS)

bin/test_obj/test_math_utils.o: src/tests/test_math_utils.c src/tests/headers/test_math_utils.h
	$(CC) $(CFLAGS) -o bin/test_obj/test_math_utils.o -c src/tests/test_math_utils.c -DRUN_TESTS $(LDFLAGS)

bin/test_obj/neuron.o: src/neuron.c src/headers/neuron.h
	$(CC) $(CFLAGS) -o bin/test_obj/neuron.o -c src/neuron.c -DRUN_TESTS $(LDFLAGS)

bin/test_obj/file_array.o: src/file_array.c src/headers/file_array.h
	$(CC) $(CFLAGS) -o bin/test_obj/file_array.o -c src/file_array.c -DRUN_TESTS $(LDFLAGS)

bin/test_obj/adj_matrix.o: src/adj_matrix.c src/headers/adj_matrix.h
	$(CC) $(CFLAGS) -o bin/test_obj/adj_matrix.o -c src/adj_matrix.c -DRUN_TESTS $(LDFLAGS)

bin/test_obj/math_utils.o: src/math_utils.c src/headers/math_utils.h
	$(CC) $(CFLAGS) -o bin/test_obj/math_utils.o -c src/math_utils.c -DRUN_TESTS $(LDFLAGS)

clean:
	rm -d -r bin
