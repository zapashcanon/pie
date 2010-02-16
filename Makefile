OBJS = pie.o ellipse.o
LIBS = -lcairo -lm
CFLAGS = -g -O0 -I/usr/include/cairo

pie: $(OBJS)
	$(CC) -o pie $(OBJS) $(LIBS)

clean:
	rm -f pie $(OBJS)
