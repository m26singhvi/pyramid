# project name (generate executable with this name)
TARGET   = common_lib.a

CC       = gcc
LINKER   = ar
# linking flags here
LFLAGS   = -Wall -I. 

# change these to set the proper directories where each files shoould be
SRCDIR   = src
OBJDIR   = obj
BINDIR   = bin

SOURCES  := $(wildcard $(SRCDIR)/*.c)
INCLUDES := $(wildcard $(SRCDIR)/*.h) 
OBJECTS  := $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
LIBSOURCES := $(wildcard $(COMMONDIR)/*.c)

rm       = rm -f

# compiling flags here
CFLAGS   = -std=c99 -Wall -Werror -Wextra -O2 -g

$(BINDIR)/$(TARGET): $(OBJECTS)
	@mkdir -p bin/
	@ar rcs $(BINDIR)/$(TARGET)  $(OBJECTS) 
	@echo "Linking complete!"

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.c $(LIBS)
	@mkdir -p obj/
	@$(CC) $(CFLAGS) -c $< -o $@  
	@echo "Compiled "$<" successfully!"

$(LIBS): 
	@make -C $(COMMONDIR)

.PHONEY: sanity
sanity:
	@$(CC) ut/main_ut.c bin/common_lib.a -I$(SRCDIR) -o ut/ut 
	@./ut/ut
	@echo "UT complete!"



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
