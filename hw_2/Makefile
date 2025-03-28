# MakeFile
# Rhiannon Kilduff
# COSC 3750 Spring 2025
# Homework 4 Makefile
# February 18, 2025 
# Makefile that compiles the given files.


CC=gcc
CFLAGS=-ggdb -Wall

# PHONY target
.PHONY: tidy clean


# Target for executable 
wycat: wycat.c
	$(CC) $(CFLAGS) wycat.c -o wycat 

#Tidy target (removes object files)
tidy: 
	/bin/rm -f a.out core.* 
clean: tidy 
	/bin/rm -f wycat