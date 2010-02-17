OBJS = pie.o ellipse.o main.o
LIBS = -lcairo -lpixman-1 -lm
CFLAGS = -Wall -g -O0 -I/usr/include/cairo

pie: $(OBJS)
	$(CC) -o pie $(OBJS) $(LIBS)

clean:
	rm -f pie $(OBJS)
