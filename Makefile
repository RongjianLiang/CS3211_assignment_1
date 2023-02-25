CC = clang-15
CXX = clang++-15

CFLAGS := $(CFLAGS) -g -O3 -Wall -Wextra -pedantic -Werror -std=c18 -pthread -fsanitize=thread
CXXFLAGS := $(CXXFLAGS) -g -O3 -Wall -Wextra -Werror -pedantic  -std=c++20 -pthread -fsanitize=thread

BUILDDIR = build

SRCS = main.cpp engine.cpp io.cpp OrderBook.cpp RestOrder.cpp BookShelf.cpp

all: engine client

engine: $(SRCS:%=$(BUILDDIR)/%.o)
	$(LINK.cc) $^ $(LOADLIBES) $(LDLIBS) -o $@

client: $(BUILDDIR)/client.cpp.o
	$(LINK.cc) $^ $(LOADLIBES) $(LDLIBS) -o $@

.PHONY: clean
clean:
	rm -rf $(BUILDDIR)
	rm -f client engine

DEPFLAGS = -MT $@ -MMD -MP -MF $(BUILDDIR)/$<.d
COMPILE.cpp = $(CXX) $(DEPFLAGS) $(CXXFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c

$(BUILDDIR)/%.cpp.o: %.cpp | $(BUILDDIR)
	$(COMPILE.cpp) $(OUTPUT_OPTION) $<

$(BUILDDIR): ; @mkdir -p $@

DEPFILES := $(SRCS:%=$(BUILDDIR)/%.d) $(BUILDDIR)/client.cpp.d

-include $(DEPFILES)
