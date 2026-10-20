.PHONY: clean

main: main.c
	gcc main.c -o main -Iinclude -lmariadb -lraylib -lm

clean:
	rm main
