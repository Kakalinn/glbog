gcc box_jumper.c $(pkg-config --libs --cflags opengl glfw3 glew) -g -lm -DMOUSE_MOVEMENT $@
