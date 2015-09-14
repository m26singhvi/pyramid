# project name (generate executable with this name)

CC       = gcc
LINKER   = gcc -o
# linking flags here
LFLAGS   = -Wall -I. -lm

# change these to set the proper directories where each files shoould be
SRCDIR   = .
OBJDIR   = . 
BINDIR   = .

SOURCES  := $(wildcard $(SRCDIR)/*.c)
INCLUDES := $(wildcard $(SRCDIR)/*.h) 
OBJECTS  := $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
rm       = rm -f

# compiling flags here
CFLAGS   = -std=c99 -Wall -Werror -Wextra -O2 -I.

libtlv.a : tlv.c
	gcc -c tlv.c -I.  
	@ar -rcs libtlv.a tlv.o

.PHONEY: clean
clean:
	@rm -rf *.a
	@rm -rf *.o
	@echo "Cleanup complete!"

