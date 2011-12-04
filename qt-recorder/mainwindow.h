#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QSlider>
#include <QtGui>

#include "dbmeter.h"

#include "AudioRecorder.hpp"

class QAction;
class QLCDNumber;


class MainWindow : public QMainWindow
{
  Q_OBJECT

  public:
    MainWindow();

    QSize sizeHint() const {
      return QSize(600, 400);
    }

    private slots:
      void openfile();
    void about();
    void record_stop();
    void stop();
    void record();
    void event_loop();

  private:

    void setupActions();
    void setupMenus();
    void setupUi();
    void stopped();
    static void rmscallback(float* values, size_t size, void* userdata);
    void rmscallback_m(float* values, size_t size, void* userdata);


    dBMeter *dbm;

    QAction *recordAction;
    QAction *openAction;
    QAction *addFilesAction;
    QAction *exitAction;
    QAction *aboutAction;
    QAction *aboutQtAction;
    QLCDNumber *timeLcd;
    QLabel *filenameLabel;
    QLabel *infoLabel;
    QString filepath;
    AudioRecorder* recorder;
    QTimer event_loop_timer;
    bool recording;
    bool current_time_advance_;
};

#endif
