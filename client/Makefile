# project name (generate executable with this name)
TARGET   = client

CC       = gcc
LINKER   = gcc -o
# linking flags here
LFLAGS   = -Wall -I. -lm

# change these to set the proper directories where each files shoould be
SRCDIR   = src
OBJDIR   = obj
BINDIR   = bin
COMMONDIR= ../common/src
COMMONDIR_OBJ= ../common/bin

SOURCES  := $(wildcard $(SRCDIR)/*.c)
INCLUDES := $(wildcard $(SRCDIR)/*.h) 
OBJECTS  := $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
LIBOBJECTS := $(wildcard $(COMMONDIR_OBJ)/*.a)

rm       = rm -f

# compiling flags here
CFLAGS   = -std=gnu99 -Wall -Werror -Wextra -O2 -I$(COMMONDIR) -g

$(BINDIR)/$(TARGET): $(OBJECTS)
	@mkdir -p bin/
	@$(LINKER) $@ $(LFLAGS) $(OBJECTS) $(LIBOBJECTS)
	@echo "Linking complete!"

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.c $(LIBS)
	@mkdir -p obj/
	@$(CC) $(CFLAGS) -c $< -o $@  
	@echo "Compiled "$<" successfully!"

$(LIBS): 
	@make -C $(COMMONDIR)

.PHONEY: clean
clean:
	@rm -rf obj
	@rm -rf bin
	@echo "Cleanup complete!"

.PHONEY: remove
remove: clean
	@rm -rf bin
	@$(rm) $(BINDIR)/$(TARGET)
	@echo "Executable removed!"
