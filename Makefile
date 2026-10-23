.PHONY: clean

visualizador: main.c
	gcc main.c -o visualizador -Iinclude -lmariadb -lraylib -lm

clean:
	rm visualizador
