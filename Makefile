
CPPFILES=CumulativeDistributionFunction.cpp Galaxy.cpp GalaxyWnd.cpp Helper.cpp main.cpp SDLWnd.cpp TextBuffer.cpp
OBJFILES=$(CPPFILES:.cpp=.o)
OUT=galaxy_renderer
PKG_CONFIG?=pkg-config
SDLLIBS:=$(shell $(PKG_CONFIG) sdl2 --libs)
SDLCFLAGS:=$(shell $(PKG_CONFIG) sdl2 --cflags)
GLEWLIBS:=$(shell $(PKG_CONFIG) --libs-only-l glew)
GLEWCFLAGS:=$(shell $(PKG_CONFIG) --cflags glew)
DEBUG=-g
CFLAGS=${DEBUG} -Wall -Wextra --pedantic -std=c++11 -Idependencies/glm -Iinclude -I/usr/include/SDL2 ${GLEWCFLAGS} ${SDLCFLAGS}
LIBS=${SDLLIBS} ${GLEWLIBS} -lSDL2_ttf

COMPILE=g++ ${CFLAGS}


$(OUT):	$(OBJFILES)
	$(COMPILE) -o $@ $^ ${LIBS}

%.o:	%.cpp
	$(COMPILE) -c -o $@ $^

clean:
	rm -f ${OBJFILES} $(OUT)

