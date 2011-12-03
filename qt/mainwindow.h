 #ifndef MAINWINDOW_H
 #define MAINWINDOW_H

 #include <QMainWindow>
 #include <QTimer>
 #include <phonon/audiooutput.h>
 #include <phonon/seekslider.h>
 #include <phonon/mediaobject.h>
 #include <phonon/volumeslider.h>
 #include <phonon/backendcapabilities.h>
// #include <QList>
 #include <QSlider>
 #include "dbmeter.h"

#include "AudioPlayer.hpp"

 class QAction;
 class QLCDNumber;


 class MainWindow : public QMainWindow
 {
     Q_OBJECT

 public:
     MainWindow();

     QSize sizeHint() const {
         return QSize(600, 500);
     }

 private slots:
     void openfile();
     void about();
     void tick(qint64 time);
     void dtest();
     void playpause();
     void stop();
     void event_loop();
     void seek(int where);

 private:
     void pause();
     void play();
     void setupActions();
     void setupMenus();
     void setupUi();
     void unload();
     void stopped();
     static void rmscallback(float* values, size_t size, void* userdata);
     void rmscallback_m(float* values, size_t size, void* userdata);

     dBMeter *dbm;

     QSlider *seekSlider;
     QSlider *volumeSlider;

     QAction *playAction;
     QAction *openAction;
     QAction *nextAction;
     QAction *previousAction;
     QAction *addFilesAction;
     QAction *exitAction;
     QAction *aboutAction;
     QAction *aboutQtAction;
     QLCDNumber *timeLcd;
     QString filepath;
     AudioPlayer* player;
     QTimer event_loop_timer;
     bool playing;
     bool current_time_advance_;
 };

 #endif
