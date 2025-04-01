OUTDIR=../bin/Debug/net8.0
OBJDIR=obj
SRCDIR=src

CC=g++
RM=rm
LINKER=g++
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
	-std=c++17\
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
