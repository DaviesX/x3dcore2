all:
	glslangValidator shaders/mvp.vert -V -o shaders/vert.spv
	glslangValidator shaders/gouraud.frag -V -o shaders/frag.spv
	g++ -O2 -std=c++17 image.cpp -o im -lvulkan -lGL -lglut -lGLEW -lglfw
	./im
