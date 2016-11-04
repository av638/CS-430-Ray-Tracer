all:	main.c
	gcc main.c -o rayTracer

clean:
	rm -rf rayTracer*~
