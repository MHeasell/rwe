ifndef PREFIX
PREFIX := /usr/local
endif

ifdef MINGW
bin_suffix := .exe
endif

ifndef CXX
CXX := g++
endif

ifndef LIBPNG_CONFIG
LIBPNG_CONFIG := libpng-config
endif

make_cflags := -Wall -I$(PNGPP) -I$(PREFIX)/include $(shell $(LIBPNG_CONFIG) --cflags) $(CFLAGS)
make_ldflags := -L$(PREFIX)/lib $(shell $(LIBPNG_CONFIG) --ldflags) $(LDFLAGS)

ifndef NDEBUG
make_cflags := $(make_cflags) -g
make_ldflags := $(make_ldflags) -g
endif

deps := $(sources:.cpp=.dep)
objects := $(sources:.cpp=.o)
bins := $(sources:.cpp=$(bin_suffix))

all: $(deps) $(bins)

%$(bin_suffix): %.o
	$(CXX) -o $@ $< $(make_ldflags)

%.o: %.cpp
	$(CXX) -c -o $@ $< $(make_cflags)

%.dep: %.cpp
	$(CXX) -M $(CPPFLAGS) $(make_cflags) $< -o- | \
	  sed 's/\($*\)\.o[ :]*/\1.o $@ : /g' > $@

clean: clean-deps
	rm -f $(bins) $(objects)

clean-deps:
	rm -f $(deps)

.PHONY: all clean clean-deps
