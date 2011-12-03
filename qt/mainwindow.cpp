#include <QtGui>

#include <stdio.h>
#include <sndfile.h>
#include <string.h>
#include <vector>
#include "vagg/vagg_macros.h"
#include <portaudio.h>

#include "mainwindow.h"

MainWindow::MainWindow()
  :player(0)
  ,playing(false)
{
  setupActions();
  setupMenus();
  setupUi();
  timeLcd->display("00:00");

  connect(&event_loop_timer, SIGNAL(timeout()), this, SLOT(event_loop()));
  connect(seekSlider, SIGNAL(valueChanged(int)), this, SLOT(seek(int)));
}

void MainWindow::openfile()
{
  unload();
  QString file = QFileDialog::getOpenFileName(this, tr("Select a .wav audio file."),
      QDesktopServices::storageLocation(QDesktopServices::MusicLocation),
      tr("Wav file (*.wav)"));

  filepath = file;
  player = new AudioPlayer(4096);
  QByteArray ba = filepath.toLocal8Bit();
  const char *c_str = ba.data();
  player->load(c_str);
  playAction->setDisabled(false);

  filepath = file;

  stopAction->setDisabled(false);
  seekSlider->setDisabled(false);
}

void MainWindow::playpause()
{
  if (player) {
    if (!playing) {
      play();
    } else {
      pause();
    }
  }
}

void MainWindow::play()
{
  player->play();
  event_loop_timer.start(50);
  playing = true;
  playAction->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
  stopAction->setDisabled(false);
}

void MainWindow::pause()
{
  player->pause();
  event_loop_timer.stop();
  playing = false;
  playAction->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
}

void MainWindow::seek(int seek)
{
  if (player) {
    double pos = seek * player->duration() / seekSlider->maximum();
    if (playing) {
      player->pause();
    }
    player->seek(pos);
    if (playing) {
      player->play();
    }
  }
}

void MainWindow::stop()
{
  player->pause();
  player->seek(0);
  playAction->setDisabled(false);
  stopped();
  seekSlider->setDisabled(true);
}

void MainWindow::unload()
{
  stopAction->setDisabled(true);
  playing = false;
  if (player) {
    delete player;
  }
  player = 0;
  stopped();
}

void MainWindow::stopped()
{
  playAction->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
  playing = false;
  stopAction->setDisabled(true);
  seekSlider->setDisabled(true);
}

void MainWindow::event_loop()
{
  if (player && ! player->state_machine()) {
    event_loop_timer.stop();
    stop();
  }
}

void MainWindow::dtest(){
  //QMessageBox::information(this, tr("Test Button!"),tr("You pressed it."));
  timeLcd->display("23:42");
  dbm->newval(qrand()%200);
  /*
     AudioBuffer ab;
     read_file("../assets/amen.wav",ab);
     */
}

void MainWindow::about()
{
  QMessageBox::information(this, tr("About Music Player"),
      tr("This player is a demo for DT2410 Lab"));
}

void MainWindow::tick(qint64 time)
{
  QTime displayTime(0, (time / 60000) % 60, (time / 1000) % 60);

  timeLcd->display(displayTime.toString("mm:ss"));
}

void MainWindow::setupActions()
{
  openAction = new QAction(style()->standardIcon(QStyle::SP_ArrowUp), tr("Test"), this);
  openAction->setDisabled(false);
  playAction = new QAction(style()->standardIcon(QStyle::SP_MediaPlay), tr("Play"), this);
  playAction->setShortcut(tr("Ctrl+P"));
  playAction->setDisabled(true);
  stopAction = new QAction(style()->standardIcon(QStyle::SP_MediaStop), tr("Stop"), this);
  stopAction->setShortcut(tr("Ctrl+S"));
  stopAction->setDisabled(true);
  nextAction = new QAction(style()->standardIcon(QStyle::SP_MediaSkipForward), tr("Next"), this);
  nextAction->setShortcut(tr("Ctrl+N"));
  previousAction = new QAction(style()->standardIcon(QStyle::SP_MediaSkipBackward), tr("Previous"), this);
  previousAction->setShortcut(tr("Ctrl+R"));
  addFilesAction = new QAction(tr("Add &Files"), this);
  addFilesAction->setShortcut(tr("Ctrl+F"));
  exitAction = new QAction(tr("E&xit"), this);
  exitAction->setShortcuts(QKeySequence::Quit);
  aboutAction = new QAction(tr("A&bout"), this);
  aboutAction->setShortcut(tr("Ctrl+B"));
  aboutQtAction = new QAction(tr("About Qt"), this);

  connect(playAction, SIGNAL(triggered()), this, SLOT(playpause()));
  connect(stopAction, SIGNAL(triggered()), this, SLOT(stop()));

  connect(addFilesAction, SIGNAL(triggered()), this, SLOT(openfile()));
  connect(exitAction, SIGNAL(triggered()), this, SLOT(close()));
  connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));
  connect(aboutQtAction, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
  connect(openAction, SIGNAL(triggered()), this, SLOT(openfile()));

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

  bar->addAction(playAction);
  bar->addAction(stopAction);
  bar->addAction(openAction);

  seekSlider = new QSlider(Qt::Horizontal, this);
  seekSlider->setFocusPolicy(Qt::StrongFocus);
  seekSlider->setTickPosition(QSlider::TicksBothSides);
  seekSlider->setTickInterval(10);
  seekSlider->setSingleStep(1);
  seekSlider->setRange(0,100);

  dbm = new dBMeter(this);
  //     connect(testAction, SIGNAL(triggered()), dbm, SLOT(newval(55)));
  //  connect(testAction, SIGNAL(triggered()), dbm, SLOT(newval(int)));
  /*
     volumeSlider = new Phonon::VolumeSlider(this);
     volumeSlider->setAudioOutput(audioOutput);
     volumeSlider->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
     */

  volumeSlider = new QSlider(Qt::Horizontal, this);
  volumeSlider->setFocusPolicy(Qt::StrongFocus);
  volumeSlider->setTickPosition(QSlider::TicksBothSides);
  volumeSlider->setTickInterval(10);
  //volumeSlider->setSingleStep(1);
  volumeSlider->setRange(0,100);

  QLabel *volumeLabel = new QLabel;
  volumeLabel->setPixmap(QPixmap("images/volume.png"));

  QPalette palette;
  palette.setBrush(QPalette::Light, Qt::darkGray);

  timeLcd = new QLCDNumber;
  timeLcd->setPalette(palette);

  QStringList headers;
  headers << tr("Title");

  QHBoxLayout *dbmLayout = new QHBoxLayout;
  dbmLayout->addWidget(dbm);

  connect(volumeSlider, SIGNAL(valueChanged(int)), dbm, SLOT(newval(int)));

  QHBoxLayout *seekerLayout = new QHBoxLayout;
  seekerLayout->addWidget(seekSlider);
  seekerLayout->addWidget(timeLcd);

  QHBoxLayout *playbackLayout = new QHBoxLayout;
  playbackLayout->addWidget(bar);
  playbackLayout->addStretch();
  playbackLayout->addWidget(volumeLabel);
  playbackLayout->addWidget(volumeSlider);

  QHBoxLayout *mid = new QHBoxLayout;
  mid->addLayout(dbmLayout);

  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addLayout(mid);
  mainLayout->addLayout(seekerLayout);
  mainLayout->addLayout(playbackLayout);

  QWidget *widget = new QWidget;
  widget->setLayout(mainLayout);

  setCentralWidget(widget);
  setWindowTitle("player wtf");
}
