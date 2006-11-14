#
#  Hand-written Makefile until we have an auto*-generated one :-)
#

CC	=	g++ 

#Release Version
CFLAGS	=	-O3 -fPIC -ffast-math -fexpensive-optimizations

#Debug-Version
#CFLAGS	=	-g -Wall 

OBJ      = 	kstfilter_generic.o

INCLUDE_DIR	   =  -I. -I/usr/local/kde/include
INCLUDE_DIR	   =  -I. -I/usr/lib/qt-3.3/include
LINK_LIBS          =  -lm -L/usr/local/kde/lib -lqt-mt -lc
LINK_LIBS          =  -lm -L/usr/lib/qt-3.3/lib -lqt-mt -lc
KST_PLUGIN         = kstfilter_generic.so

all:	$(KST_PLUGIN)


$(KST_PLUGIN): $(OBJ) 
	@echo $(CC) -shared -o $(KST_PLUGIN) $(OBJ) $(LINK_LIBS)
	@$(CC) -shared -z defs -o $(KST_PLUGIN) $(OBJ) $(LINK_LIBS)

###  rules #########################################

%.o:	%.cpp
	$(CC) $(CFLAGS) $(INCLUDE_DIR) -c $< -o $@

clean:
	@echo -n "cleaning up..."	
	@rm -f $(KST_PLUGIN) *.[ao] *~ &> /dev/null
	@rm -f core &> /dev/null
	@rm -f $(OBJ)
	@echo    "done"

new:	
	$(MAKE) clean
	$(MAKE) all
