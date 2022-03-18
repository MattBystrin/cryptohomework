EXEC = task
BDIR = build
SRCS = main.cpp
FLGS = -g -std=c++17
LIBS = `pkg-config --libs icu-i18n`
OBJS = $(patsubst $(notdir %.cpp),%.o,$(SRCS))

.PHONY: all clean

all: dir executable

clean:
	@rm $(BDIR)/*

$(BDIR)/%.o : %.cpp
	g++ $(FLGS) -c $^ -o $@

executable: $(addprefix $(BDIR)/,$(OBJS))
	g++ -o $(EXEC) $^ $(LIBS)

dir:
	@mkdir -p $(BDIR)
