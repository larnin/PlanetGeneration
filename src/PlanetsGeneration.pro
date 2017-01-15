#-------------------------------------------------
#
# Project created by QtCreator 2017-01-10T13:08:10
#
#-------------------------------------------------

QT       -= core gui

TARGET = PlanetsGeneration
TEMPLATE = app


SOURCES += main.cpp \
    utilities.cpp \
    spheresurface.tpp \
    generator.cpp \
    block.cpp \
    render.cpp

HEADERS  += \
    sphericaldistribution.h \
    spheresurface.h \
    vectconvert.h \
    block.h \
    spherepoint.h \
    utilities.h \
    generator.h \
    render.h

CONFIG += c++14
#QMAKE_CXXFLAGS += -Wsign-conversion

# SFML
DEFINES += SFML_STATIC
#   --- HOME
LIBS += -LC:/Users/Nicolas/Programation/c++/SFML/DW2_2.4_(Qt)/lib
INCLUDEPATH += C:/Users/Nicolas/Programation/c++/SFML/DW2_2.4_(Qt)/include
DEPENDPATH += C:/Users/Nicolas/Programation/c++/SFML/DW2_2.4_(Qt)/include

#   --- RUBIKA
#LIBS += -LC:/Users/n.laurent/Desktop/perso/SFML/SFML-2.4.0-SW2/lib
#INCLUDEPATH += C:/Users/n.laurent/Desktop/perso/SFML/SFML-2.4.0-SW2/include
#DEPENDPATH += C:/Users/n.laurent/Desktop/perso/SFML/SFML-2.4.0-SW2/include

CONFIG(release, debug|release): LIBS += -lsfml-graphics-s \
                                        -lsfml-window-s \
                                        -lsfml-audio-s \
                                        -lsfml-system-s \
                                        -lfreetype \
                                        -lgdi32 \
                                        #-lglew \
                                        -ljpeg \
                                        -lopengl32 \
                                        -lwinmm \
                                        -lopenal32 \
                                        -lflac \
                                        -lvorbisenc \
                                        -lvorbisfile \
                                        -lvorbis \
                                        -logg

CONFIG(debug, debug|release): LIBS +=   -lsfml-graphics-s-d \
                                        -lsfml-window-s-d \
                                        -lsfml-audio-s-d \
                                        -lsfml-system-s-d \
                                        -lfreetype \
                                        -lgdi32 \
                                        #-lglew \
                                        -ljpeg \
                                        -lopengl32 \
                                        -lwinmm \
                                        -lopenal32 \
                                        -lflac \
                                        -lvorbisenc \
                                        -lvorbisfile \
                                        -lvorbis \
                                        -logg
