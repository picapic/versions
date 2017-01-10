#
# filestatus makefile
#

GCC=g++
PRJNAME=file_status

all:
	$(GCC) -o ./$(PRJNAME) $(PRJNAME).cpp
