all : 
	gcc  histogram.c -o histogram

clean:
	-rm -f histogram
