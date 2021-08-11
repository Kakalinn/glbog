gcc moving_box.c $(pkg-config --libs --cflags opengl glfw3 glew) -g $@
