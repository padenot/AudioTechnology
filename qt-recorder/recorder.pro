HEADERS   += mainwindow.h \
             ../qt-player/dbmeter.h \
             ../src/AudioFile.hpp \
             ../src/AudioRecorder.hpp \
             ../src/RingBuffer.hpp \
             ../src/Effect.hpp \
             ../src/RMS.hpp

SOURCES   += main.cpp \
             ../qt-player/dbmeter.cpp \
             mainwindow.cpp \
             ../src/AudioFile.cpp \
             ../src/AudioRecorder.cpp

CONFIG += debug
QMAKE_CXXFLAGS += -std=c++0x -DVAGG_DEBUG
LIBS      += -lvagg -L../vagg -lsndfile -lrt -lasound -lpthread -lportaudio -Wl,-rpath -Wl,/usr/local/lib/ -Wl,-rpath -Wl,../vagg -L/usr/local/lib -I. -Lvagg -lvagg  -lm
INCLUDEPATH += ../
INCLUDEPATH += ../src

# install
target.path = $$[QT_INSTALL_EXAMPLES]/phonon/qmusicplayer
sources.files = $$SOURCES $$HEADERS $$FORMS $$RESOURCES *.pro *.png images
sources.path = $$[QT_INSTALL_EXAMPLES]/phonon/qmusicplayer
INSTALLS += target sources

wince*{
DEPLOYMENT_PLUGIN += phonon_ds9 phonon_waveout
}
