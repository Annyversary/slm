CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2

TARGET   := slm
SRCDIR   := src
OBJDIR   := obj

# Automatically find all .cpp source files under src/
SRCS := $(wildcard $(SRCDIR)/*.cpp)
OBJS := $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SRCS))

# ----------------------------------------------------------------
# Default target: build the executable
# ----------------------------------------------------------------
.PHONY: all
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile each source file, creating obj/ if it does not exist.
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJDIR):
	mkdir -p $(OBJDIR)

# ----------------------------------------------------------------
# Clean: remove all generated files
# ----------------------------------------------------------------
.PHONY: clean
clean:
	rm -rf $(OBJDIR) $(TARGET)
