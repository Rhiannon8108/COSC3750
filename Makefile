# MakeFile
# Rhiannon Kilduff
# COSC 3750 Spring 2025
# Homework 6 Makefile
# March 28th, 2025 
# Makefile that compiles the given files.


CC=gcc
CFLAGS=-ggdb -Wall

# PHONY target
.PHONY: tidy clean


# Target for executable 
wytar: wytar.c tarFunctions.c
	$(CC) $(CFLAGS) wytar.c tarFunctions.0 -o wytar 

tarFunctions.0 tarFunctions.c tarFunctions.h 
	$(CC) $(CFLAGS) -c tarFunctions.c

#Tidy target (removes object files)
tidy: 
	/bin/rm -f  tarFunctions.0 a.out core.* 
clean: tidy 
	/bin/rm -f wytar
