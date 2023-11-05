COMPILER = g++
OUTPUT = bin
FLAGS = --std=c++11 -O -Wall

CAIRO_FLAGS = -lcairo -lX11
OPENGL_FLAGS = -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl
RENDERER = cairo_renderer
RENDERER_FLAGS = $(CAIRO_FLAGS)

all: main.o softbody.o edge.o node.o id.o vectors.o ui.o base_renderer.o $(RENDERER).o simulator.o
	$(COMPILER) $(FLAGS) $(RENDERER_FLAGS) -o $(OUTPUT) main.o softbody.o edge.o node.o vectors.o ui.o base_renderer.o $(RENDERER).o simulator.o

main.o: main.cpp softbody.o edge.o node.o vectors.o
	$(COMPILER) $(FLAGS) -c main.cpp

simulator.o: simulator.cpp simulator.h vectors.o softbody.o node.o edge.o;
	$(COMPILER) $(FLAGS) -c simulator.cpp

softbody.o: softbody/softbody.cpp softbody/softbody.h edge.o vectors.o id.o
	$(COMPILER) $(FLAGS) -c softbody/softbody.cpp

edge.o: softbody/edge.cpp softbody/edge.h node.o vectors.o
	$(COMPILER) $(FLAGS) -c softbody/edge.cpp

node.o: softbody/node.cpp softbody/node.h vectors.o;
	$(COMPILER) $(FLAGS) -c softbody/node.cpp

id.o: utils/id.cpp;
	$(COMPILER) $(FLAGS) -c utils/id.cpp

vectors.o: utils/vectors.cpp;
	$(COMPILER) $(FLAGS) -c utils/vectors.cpp

ui.o: ui/ui.cpp $(RENDERER).o;
	$(COMPILER) $(FLAGS) -c ui/ui.cpp

opengl_renderer.o: ui/renderers.h ui/opengl_renderer.cpp base_renderer.o;
	$(COMPILER) $(FLAGS) $(OPENGL_FLAGS) -c ui/opengl_renderer.cpp

cairo_renderer.o: ui/renderers.h ui/cairo_renderer.cpp base_renderer.o;
	$(COMPILER) $(FLAGS) $(CAIRO_FLAGS) -c ui/cairo_renderer.cpp

base_renderer.o: ui/renderers.h ui/base_renderer.cpp;
	$(COMPILER) $(FLAGS) -c ui/base_renderer.cpp



clean:
	rm -f main.o simulator.o edge.o node.o softbody.o vectors.o id.o base_renderer.o cairo_renderer.o ui.o opengl_renderer.o
