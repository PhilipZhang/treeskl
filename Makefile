###########################################

# File Name : makefile

# Purpose : Makefile for

# Creation Date : 10-05-2013

# Last Modified : Wed 22 May 2013 10:22:00 PM CST

# Created By : Philip Zhang 

############################################

MAIN = SklPresent
SRCPATH = ./src/
SHAREDPATH = ./shared/
SHAREDINCPATH = ./include/
LIBDIRS = -L/usr/X11R6/lib -L/usr/X11R6/lib64 -L/usr/local/lib
INCDIRS = -I/usr/include -I/usr/local/include -I/usr/include/GL -I$(SHAREDINCPATH) -I$(SRCPATH)
CC = g++
CFLAGS = $(COMPILEFLAGS) -g $(INCDIRS)
LIBS = -lX11 -lglut -lGL -lGLU -lm -lGLEW

prog: $(MAIN)

Main.o : $(SRCPATH)Main.cpp
TreeSkeleton.o : $(SRCPATH)TreeSkeleton.cpp
TreePointCloud.o : $(SRCPATH)TreePointCloud.cpp
VoxelModel.o : $(SRCPATH)VoxelModel.cpp
GLUtils.o	: $(SHAREDPATH)GLUtils.cpp
GLTools.o : $(SHAREDPATH)GLTools.cpp
GLBatch.o : $(SHAREDPATH)GLBatch.cpp
GLTriangleBatch.o : $(SHAREDPATH)GLTriangleBatch.cpp
GLShaderManager.o : $(SHAREDPATH)GLShaderManager.cpp
math3d.o	: $(SHAREDPATH)math3d.cpp

$(MAIN) : Main.o TreeSkeleton.o TreePointCloud.o GLUtils.o VoxelModel.o
	$(CC) $(CFLAGS) -o $(MAIN) $(LIBDIRS) $(SRCPATH)Main.cpp $(SRCPATH)TreeSkeleton.cpp $(SRCPATH)TreePointCloud.cpp $(SRCPATH)VoxelModel.cpp $(SHAREDPATH)GLUtils.cpp $(SHAREDPATH)GLTools.cpp $(SHAREDPATH)GLBatch.cpp $(SHAREDPATH)GLTriangleBatch.cpp $(SHAREDPATH)GLShaderManager.cpp $(SHAREDPATH)math3d.cpp $(LIBS)

clean:
	rm -f *.o

edit:
	vi -p src/*.cpp src/*.h

run:
	./$(MAIN)

add:
	git add src/* shared/* include/* skls/* shaders/* textures/*
	git add *
