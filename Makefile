
.POSIX:

CC = gcc
CFLAGS = -std=c11 -g
LDFLAGS = -lm
MKDIR = mkdir -p

.PHONY: dirs

all: dirs bin/neuralnet bin/test_neuralnet

dirs: bin bin/obj bin/test_obj output images
bin:
	$(MKDIR) bin
bin/obj:
	$(MKDIR) bin/obj
bin/test_obj:
	$(MKDIR) bin/test_obj
output:
	$(MKDIR) output
images:
	$(MKDIR) images 

bin/neuralnet: bin/obj/main.o bin/obj/neuron.o bin/obj/file_table.o bin/obj/adj_matrix.o bin/obj/math_utils.o bin/obj/timer.o
	$(CC) $(CFLAGS) -o bin/neuralnet bin/obj/main.o bin/obj/neuron.o bin/obj/file_table.o bin/obj/adj_matrix.o bin/obj/math_utils.o bin/obj/timer.o $(LDFLAGS)

bin/obj/main.o: src/main.c
	$(CC) $(CFLAGS) -o bin/obj/main.o -c src/main.c $(LDFLAGS)

bin/obj/neuron.o: src/neuron.c src/headers/neuron.h
	$(CC) $(CFLAGS) -o bin/obj/neuron.o -c src/neuron.c $(LDFLAGS)

bin/obj/file_table.o: src/file_table.c src/headers/file_table.h
	$(CC) $(CFLAGS) -o bin/obj/file_table.o -c src/file_table.c $(LDFLAGS)

bin/obj/adj_matrix.o: src/adj_matrix.c src/headers/adj_matrix.h
	$(CC) $(CFLAGS) -o bin/obj/adj_matrix.o -c src/adj_matrix.c $(LDFLAGS)

bin/obj/math_utils.o: src/math_utils.c src/headers/math_utils.h
	$(CC) $(CFLAGS) -o bin/obj/math_utils.o -c src/math_utils.c $(LDFLAGS)

bin/obj/timer.o: src/timer.c src/headers/timer.h
	$(CC) $(CFLAGS) -o bin/obj/timer.o -c src/timer.c $(LDFLAGS)

bin/test_neuralnet: bin/test_obj/test.o bin/test_obj/test_utils.o bin/test_obj/test_file_table.o bin/test_obj/test_adj_matrix.o bin/test_obj/test_math_utils.o bin/test_obj/test_timer.o bin/test_obj/neuron.o bin/test_obj/file_table.o bin/test_obj/adj_matrix.o bin/test_obj/math_utils.o bin/test_obj/timer.o
	$(CC) $(CFLAGS) -o bin/test_neuralnet bin/test_obj/test.o bin/test_obj/test_utils.o bin/test_obj/test_file_table.o bin/test_obj/test_adj_matrix.o bin/test_obj/test_math_utils.o bin/test_obj/test_timer.o bin/test_obj/neuron.o bin/test_obj/file_table.o bin/test_obj/adj_matrix.o bin/test_obj/math_utils.o bin/test_obj/timer.o $(LDFLAGS)

bin/test_obj/test.o: src/tests/test.c src/tests/headers/test_utils.h
	$(CC) $(CFLAGS) -o bin/test_obj/test.o -c src/tests/test.c -DRUN_TESTS $(LDFLAGS)

bin/test_obj/test_utils.o: src/tests/test_utils.c src/tests/headers/test_utils.h
	$(CC) $(CFLAGS) -o bin/test_obj/test_utils.o -c src/tests/test_utils.c -DRUN_TESTS $(LDFLAGS)

bin/test_obj/test_file_table.o: src/tests/test_file_table.c src/tests/headers/test_file_table.h
	$(CC) $(CFLAGS) -o bin/test_obj/test_file_table.o -c src/tests/test_file_table.c -DRUN_TESTS $(LDFLAGS)

bin/test_obj/test_adj_matrix.o: src/tests/test_adj_matrix.c src/tests/headers/test_adj_matrix.h
	$(CC) $(CFLAGS) -o bin/test_obj/test_adj_matrix.o -c src/tests/test_adj_matrix.c -DRUN_TESTS $(LDFLAGS)

bin/test_obj/test_math_utils.o: src/tests/test_math_utils.c src/tests/headers/test_math_utils.h
	$(CC) $(CFLAGS) -o bin/test_obj/test_math_utils.o -c src/tests/test_math_utils.c -DRUN_TESTS $(LDFLAGS)

bin/test_obj/test_timer.o: src/tests/test_timer.c src/tests/headers/test_timer.h
	$(CC) $(CFLAGS) -o bin/test_obj/test_timer.o -c src/tests/test_timer.c -DRUN_TESTS $(LDFLAGS)

bin/test_obj/neuron.o: src/neuron.c src/headers/neuron.h
	$(CC) $(CFLAGS) -o bin/test_obj/neuron.o -c src/neuron.c -DRUN_TESTS $(LDFLAGS)

bin/test_obj/file_table.o: src/file_table.c src/headers/file_table.h
	$(CC) $(CFLAGS) -o bin/test_obj/file_table.o -c src/file_table.c -DRUN_TESTS $(LDFLAGS)

bin/test_obj/adj_matrix.o: src/adj_matrix.c src/headers/adj_matrix.h
	$(CC) $(CFLAGS) -o bin/test_obj/adj_matrix.o -c src/adj_matrix.c -DRUN_TESTS $(LDFLAGS)

bin/test_obj/math_utils.o: src/math_utils.c src/headers/math_utils.h
	$(CC) $(CFLAGS) -o bin/test_obj/math_utils.o -c src/math_utils.c -DRUN_TESTS $(LDFLAGS)

bin/test_obj/timer.o: src/timer.c src/headers/timer.h
	$(CC) $(CFLAGS) -o bin/test_obj/timer.o -c src/timer.c -DRUN_TESTS $(LDFLAGS)

clean:
	rm -d -r bin output
