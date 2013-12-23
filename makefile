CPP = g++
CFLAGS = -static-libgcc -static-libstdc++ -Os -s -DWINSTYLE
REMOVE = del
COPY = copy
INSTALLDIR = D:\\ldraw\\
EXECFILE = LDMakeList.exe
OFILES = LDMakeList.o LDrawPart.o
HFILES = CmdOpts.H LDrawPart.H FileIO.H TextStuff.H

all: $(EXECFILE)
$(EXECFILE): $(OFILES) $(HFILES)
	$(CPP) $(CFLAGS) $(OFILES) -o $(EXECFILE)
$(OFILES) : %.o: %.C $(HFILES)
	$(CPP) $(CFLAGS) -c $< -o $@

install:
	$(COPY) $(EXECFILE) $(INSTALLDIR)
clean:
	$(REMOVE) $(OFILES) $(EXECFILE)
tidy:
	$(REMOVE) *~
