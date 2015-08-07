# project name (generate executable with this name)
TARGET   = server

CC       = gcc
LINKER   = gcc -o
# linking flags here
LFLAGS   = -Wall -I. -lm

# change these to set the proper directories where each files shoould be
SRCDIR   = src
OBJDIR   = obj
BINDIR   = bin
COMMONDIR= ../common

SOURCES  := $(wildcard $(SRCDIR)/*.c)
INCLUDES := $(wildcard $(SRCDIR)/*.h) 
OBJECTS  := $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
rm       = rm -f

# compiling flags here
CFLAGS   = -std=c99 -Wall  -Wextra -O2 -I$(COMMONDIR)

$(BINDIR)/$(TARGET): $(OBJECTS)
	@mkdir -p bin/
	@$(LINKER) $@ $(LFLAGS) $(OBJECTS) -lpthread
	@echo "Linking complete!"

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.c
	@mkdir -p obj/
	@$(CC) $(CFLAGS) -c $< -o $@ -lpthread
	@echo "Compiled "$<" successfully!"

.PHONEY: clean
clean:
	@rm -rf obj
	@rm -rf bin
	@echo "Cleanup complete!"

.PHONEY: remove
remove: clean
	@$(rm) $(BINDIR)/$(TARGET)
	@echo "Executable removed!"
