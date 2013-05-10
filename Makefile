###########################################

# File Name : makefile

# Purpose : Makefile for

# Creation Date : 10-05-2013

# Last Modified : Fri 10 May 2013 02:16:41 PM CST

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

$(MAIN).o : $(SRCPATH)$(MAIN).cpp
TreeSkeleton.o : $(SRCPATH)TreeSkeleton.cpp
GLTools.o : $(SHAREDPATH)GLTools.cpp
GLBatch.o : $(SHAREDPATH)GLBatch.cpp
GLTriangleBatch.o : $(SHAREDPATH)GLTriangleBatch.cpp
GLShaderManager.o : $(SHAREDPATH)GLShaderManager.cpp
math3d.o	: $(SHAREDPATH)math3d.cpp

$(MAIN) : $(MAIN).o TreeSkeleton.o
	$(CC) $(CFLAGS) -o $(MAIN) $(LIBDIRS) $(SRCPATH)$(MAIN).cpp $(SRCPATH)TreeSkeleton.cpp $(SHAREDPATH)GLTools.cpp $(SHAREDPATH)GLBatch.cpp $(SHAREDPATH)GLTriangleBatch.cpp $(SHAREDPATH)GLShaderManager.cpp $(SHAREDPATH)math3d.cpp $(LIBS)

clean:
	rm -f *.o

