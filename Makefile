CC = clang
CXX = clang++

CFLAGS_ADDRESS := $(CFLAGS) -g -O3 -Wall -Wextra -pedantic -Werror -std=c18 -pthread -fsanitize=address -fsanitize=undefined -fno-sanitize-recover=all -fsanitize=float-divide-by-zero -fsanitize=float-cast-overflow -fno-sanitize=null -fno-sanitize=alignment
CXXFLAGS_ADDRESS := $(CXXFLAGS) -g -O3 -Wall -Wextra -Werror -pedantic  -std=c++20 -pthread -fsanitize=address -fsanitize=undefined -fno-sanitize-recover=all -fsanitize=float-divide-by-zero -fsanitize=float-cast-overflow -fno-sanitize=null -fno-sanitize=alignment
CFLAGS_THREAD := $(CFLAGS) -g -O3 -Wall -Wextra -pedantic -Werror -std=c18 -pthread -fsanitize=thread
CXXFLAGS_THREAD := $(CXXFLAGS) -g -O3 -Wall -Wextra -Werror -pedantic  -std=c++20 -pthread -fsanitize=thread
CFLAGS_VALGRIND := $(CFLAGS) -g -O3 -Wall -Wextra -pedantic -Werror -std=c18 -pthread
CXXFLAGS_VALGRIND := $(CXXFLAGS) -g -O3 -Wall -Wextra -Werror -pedantic  -std=c++20 -pthread 
CFLAGS_MEMORY := $(CFLAGS) -g -O3 -Wall -Wextra -pedantic -Werror -std=c18 -pthread -fsanitize=memory
CXXFLAGS_MEMORY := $(CXXFLAGS) -g -O3 -Wall -Wextra -Werror -pedantic  -std=c++20 -pthread -fsanitize=memory
CFLAGS := $(CFLAGS_THREAD)
CXXFLAGS:= $(CXXFLAGS_THREAD)

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
