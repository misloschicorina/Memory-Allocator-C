#Misloschi Alexandra Corina 314 CA 2023-2024
CC=gcc
CFLAGS=-Wall -Wextra -std=c99

TARGETS = sfl

build: $(TARGETS)

sfl: sfl.c
	$(CC) $(CFLAGS) sfl.c -o sfl

pack:
	zip -FSr 314CA_MisloschiAlexandraCorina_Tema3.zip README Makefile *.c *.h 

clean:
	rm -f $(TARGETS)

.PHONY: pack clean