CFLAGS = -std=c++17 -I. -I include
LDFLAGS = -ldl -lglfw -lassimp

a.out: shaders/* *.hpp *.c *.cpp *h
	g++ $(CFLAGS) -o a.out *.c *.cpp  -I `pkg-config --cflags --libs opencv4` $(LDFLAGS)


.PHONY: test clean

test: a.out
	./a.out

clean:
	rm -f a.out