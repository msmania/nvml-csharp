OUTDIR=../bin/Debug/net8.0
OBJDIR=obj
SRCDIR=src

# Need G++13 for std::format
CC=g++-13
RM=rm
LINKER=g++-13
TARGET=mylib

SRCS=$(wildcard $(SRCDIR)/*.cpp)
OBJS=$(addprefix $(OBJDIR)/, $(notdir $(SRCS:.cpp=.o)))

LIBS=\
	-lpthread\
	-lnvidia-ml\

CFLAGS=\
	-c\
	-O0\
	-Wall\
	-g\
	-std=c++20\
	-fPIC\
	-I/usr/local/cuda/include\

LFLAGS=\
	-shared\

all: $(OUTDIR)/$(TARGET)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@[ -d $(OBJDIR) ] || mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -o $@ -c $<

$(OUTDIR)/$(TARGET): $(OBJS)
	@[ -d $(OUTDIR) ] || mkdir -p $(OUTDIR)
	$(LINKER) $(LFLAGS) $(LIBDIRS) $^ -o $@ $(LIBS)

clean:
	$(RM) $(OBJS) $(OUTDIR)/$(TARGET)
