all:
	g++ -o scanline scanline.cpp -std=c++11 -lGL -lGLU -lglut

run:
	./scanline