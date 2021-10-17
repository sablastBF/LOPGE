CFLAGS = -std=c++17 -I. -I include
LDFLAGS = -ldl -lglfw

a.out: *.c *.cpp 
	g++ $(CFLAGS) -o a.out *.cpp $(LDFLAGS)


.PHONY: test clean

test: a.out
	./a.out

clean:
	rm -f a.out
