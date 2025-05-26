main: main.c
	$(CC) main.c -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -o destiny_matrix