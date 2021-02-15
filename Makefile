
CC = `curl-config --cc`
LIBS = `curl-config --libs` -ljson-c
LIBES = 


curlapp: curlapp.o
	$(CC) -g -o $@ $< $(LIBS) 

curlapp.o: curlapp.c
	$(CC) -g -c $< $(LIBES)

clean:
	rm -f curlapp.o curlapp

curlapp.o: curlapp.c