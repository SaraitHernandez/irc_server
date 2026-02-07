# IRC Server Makefile
# C++98 compliant, works on Linux and Mac

NAME = ircserv
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

# Detect OS and set platform-specific flags
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	# macOS
	CXXFLAGS += -D__MACOS__
endif
ifeq ($(UNAME_S),Linux)
	# Linux
	CXXFLAGS += -D__LINUX__
endif

# Directories
SRCDIR = src
INCDIR = include/irc
OBJDIR = objs

# Source files (to be added as development progresses)
SOURCES = $(wildcard $(SRCDIR)/*.cpp) \
          $(wildcard $(SRCDIR)/commands/*.cpp)

# Object files
OBJECTS = $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

# Header files for dependency tracking
INCLUDES = $(wildcard $(INCDIR)/*.hpp) \
           $(wildcard $(INCDIR)/commands/*.hpp)

# Default target
all: $(NAME)

# Create object directory structure
$(OBJDIR):
	@mkdir -p $(OBJDIR)
	@mkdir -p $(OBJDIR)/commands
	
# Link executable (only if object files exist)
$(NAME): $(OBJDIR) $(OBJECTS)
	@if [ -z "$(OBJECTS)" ]; then \
		echo "No source files found. Please add source files to $(SRCDIR)/"; \
		exit 1; \
	fi
	@echo "Linking $(NAME)..."
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(NAME)
	@echo "$(NAME) compiled successfully"

# Compile source files to object files
# Dependencies: only recompile if source or headers changed
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) -Iinclude -c $< -o $@

# Clean object files
clean:
	@echo "Cleaning object files..."
	@rm -rf $(OBJDIR)
	@echo "Clean complete"

# Full clean (objects + executable)
fclean: clean
	@echo "Removing $(NAME)..."
	@rm -f $(NAME)
	@echo "Full clean complete"

# Rebuild (fclean + all)
re: fclean all

# Phony targets
.PHONY: all clean fclean re

