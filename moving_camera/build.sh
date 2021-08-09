gcc moving_camera.c $(pkg-config --libs --cflags opengl glfw3 glew) -g -lm $@
