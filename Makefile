CC=g++
CFLAGS=`freetype-config --cflags`
FREETYPE=`freetype-config --libs`
LIBS=$(FREETYPE)  -lpngwriter -lpng
ranvis: ranvis.c
	$(CC) $< $(INCLUDE) $(CFLAGS) $(LIBS) -o $@
