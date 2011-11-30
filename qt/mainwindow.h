 #ifndef MAINWINDOW_H
 #define MAINWINDOW_H

 #include <QMainWindow>
 #include <phonon/audiooutput.h>
 #include <phonon/seekslider.h>
 #include <phonon/mediaobject.h>
 #include <phonon/volumeslider.h>
 #include <phonon/backendcapabilities.h>
 #include <QList>
 #include <QSlider>
 #include "dbmeter.h"

 class QAction;
 class QTableWidget;
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
     void addFiles();
     void about();
     void stateChanged(Phonon::State newState, Phonon::State oldState);
     void tick(qint64 time);
     void sourceChanged(const Phonon::MediaSource &source);
     void metaStateChanged(Phonon::State newState, Phonon::State oldState);
     void aboutToFinish();
     void tableClicked(int row, int column);
     void dtest();

 private:
     void setupActions();
     void setupMenus();
     void setupUi();

     dBMeter *dbm;

     QSlider *seekSlider;
     Phonon::MediaObject *mediaObject;
     Phonon::MediaObject *metaInformationResolver;
     Phonon::AudioOutput *audioOutput;
     
     QSlider *volumeSlider;
     QList<Phonon::MediaSource> sources;

     QAction *playAction;
     QAction *testAction;
     QAction *pauseAction;
     QAction *stopAction;
     QAction *nextAction;
     QAction *previousAction;
     QAction *addFilesAction;
     QAction *exitAction;
     QAction *aboutAction;
     QAction *aboutQtAction;
     QLCDNumber *timeLcd;
     QTableWidget *musicTable;
 };

 #endif
