gcc box_jumper.c $(pkg-config --libs --cflags opengl glfw3 glew) -g -lm -DMOUSE_MOVEMENT -DWIDTH=1280 -DHEIGHT=720 $@
