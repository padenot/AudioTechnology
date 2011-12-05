#include <stdio.h>
#include <sndfile.h>
#include <string.h>
#include <vector>
#include "vagg/vagg_macros.h"
#include <portaudio.h>
#include <QDialog>

#include "RMS.hpp"
#include "mainwindow.h"

static float rms2db(float value)
{
  return 20 * log10(value);
}

void MainWindow::rmscallback(float* values, size_t size, void* user_data)
{
  MainWindow* mw = static_cast<MainWindow*>(user_data);
  for (size_t i = 0; i < size; i++) {
    values[i] = rms2db(values[i]);
  }
  mw->rmscallback_m(values, size, user_data);
}

void MainWindow::rmscallback_m(float* values, size_t size, void* user_data)
{
  dbm->valueChanged(values, size);
}

  MainWindow::MainWindow()
  :recorder(0)
   ,recording(false)
{
  setupActions();
  setupMenus();
  setupUi();
  timeLcd->display("00:00");

  connect(&event_loop_timer, SIGNAL(timeout()), this, SLOT(event_loop()));
}

void MainWindow::openfile()
{
  QString file = QFileDialog::getSaveFileName(this, tr("Save as .wav."),
      QDesktopServices::storageLocation(QDesktopServices::MusicLocation),
      tr("Wav file (*.wav)"));

  if(file != 0) {
    filepath = file;
    recorder = new AudioRecorder(4096);
    recorder->insert(new RMS(&MainWindow::rmscallback, this));

    QByteArray ba = filepath.toAscii();
    //printf("%s", ba);
    recorder->open(ba);
    recordAction->setDisabled(false);

    filepath = file;
    filenameLabel->setText(file);
  }
}

void MainWindow::record_stop()
{
  if (recorder) {
    if (!recording) {
      record();
    } else {
      stop();
    }
  }
}

void MainWindow::record()
{
  if (recorder) {
    recorder->record();
    recording = true;
    event_loop_timer.start(1);
    recordAction->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
  }
}

void MainWindow::stop()
{
  if (recorder) {
    recorder->stop();
    recordAction->setDisabled(true);
    filepath.clear();
    stopped();
  }
}

void MainWindow::stopped()
{
  recording = false;
  recordAction->setIcon(QIcon("assets/recording.svg"));
}

void MainWindow::event_loop()
{
  current_time_advance_ = true;
  double current_time = recorder->current_time();
  short seconds = static_cast<int>(current_time) % 60;
  int minutes = current_time / 60;
  QString text;
  text = text.sprintf("%02d:%02d",minutes, seconds);
  timeLcd->display(text);
  text.clear();
  double o_to_go = 1024. * 1024. * 1024.;
  double free_space = recorder->free_disk_space();
  if (free_space == -1) {
    text = text.sprintf("Could not determine remaining space for ") + filepath;
  } else {
    text = text.sprintf("%lfGo remaining.", free_space / o_to_go);
  }
  infoLabel->setText(text);
  current_time_advance_ = false;
  if (recorder && ! recorder->state_machine()) {
    event_loop_timer.stop();
    stop();
  }
}

void MainWindow::about()
{
  QMessageBox::information(this, tr("About Music recorder"),
      tr("This recorder is a demo for DT2410 Lab 2011"));
}

void MainWindow::setupActions()
{
  openAction = new QAction(style()->standardIcon(QStyle::SP_ArrowUp), tr("Open File"), this);
  openAction->setDisabled(false);
  recordAction = new QAction(QIcon("assets/recording.svg"), tr("Play"), this);
  recordAction->setShortcut(tr("Ctrl+P"));
  recordAction->setDisabled(true);
  /*
     nextAction = new QAction(style()->standardIcon(QStyle::SP_MediaSkipForward), tr("Next"), this);
     nextAction->setShortcut(tr("Ctrl+N"));
     previousAction = new QAction(style()->standardIcon(QStyle::SP_MediaSkipBackward), tr("Previous"), this);
     previousAction->setShortcut(tr("Ctrl+R"));
     */
  addFilesAction = new QAction(tr("Open &File"), this);
  addFilesAction->setShortcut(tr("Ctrl+F"));
  exitAction = new QAction(tr("E&xit"), this);
  exitAction->setShortcuts(QKeySequence::Quit);
  aboutAction = new QAction(tr("A&bout"), this);
  aboutAction->setShortcut(tr("Ctrl+B"));
  aboutQtAction = new QAction(tr("About Qt"), this);

  connect(recordAction, SIGNAL(triggered()), this, SLOT(record_stop()));

  connect(addFilesAction, SIGNAL(triggered()), this, SLOT(openfile()));
  connect(openAction, SIGNAL(triggered()), this, SLOT(openfile()));
  connect(exitAction, SIGNAL(triggered()), this, SLOT(close()));
  connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));
  connect(aboutQtAction, SIGNAL(triggered()), qApp, SLOT(aboutQt()));


}

void MainWindow::setupMenus()
{
  QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
  fileMenu->addAction(addFilesAction);
  fileMenu->addSeparator();
  fileMenu->addAction(exitAction);

  QMenu *aboutMenu = menuBar()->addMenu(tr("&Help"));
  aboutMenu->addAction(aboutAction);
  aboutMenu->addAction(aboutQtAction);
}

void MainWindow::setupUi()
{
  QToolBar *bar = new QToolBar;

  bar->addAction(recordAction);
  bar->addAction(openAction);

  QPalette palette;
  palette.setBrush(QPalette::Light, Qt::darkGray);

  timeLcd = new QLCDNumber;
  timeLcd->setPalette(palette);

  filenameLabel = new QLabel("Select a file for the recording.");
  infoLabel = new QLabel("");

  dbm = new dBMeter(this);

  QVBoxLayout *textLayout = new QVBoxLayout;
  textLayout->addWidget(filenameLabel);
  textLayout->addWidget(infoLabel);
  textLayout->addWidget(timeLcd);



  QHBoxLayout *playbackLayout = new QHBoxLayout;
  playbackLayout->addWidget(bar);
  playbackLayout->addStretch();

  QHBoxLayout *mid = new QHBoxLayout;
  mid->addLayout(textLayout);
  mid->addWidget(dbm);


  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addLayout(mid);
  mainLayout->addLayout(playbackLayout);

  QWidget *widget = new QWidget;
  widget->setLayout(mainLayout);

  setCentralWidget(widget);
  setWindowTitle("wav recorder 0.1");
}
