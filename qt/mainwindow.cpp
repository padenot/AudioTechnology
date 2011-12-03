

#include <stdio.h>
#include <sndfile.h>
#include <string.h>
#include <vector>
#include "vagg/vagg_macros.h"
#include <portaudio.h>

#include "RMS.hpp"
#include "mainwindow.h"

static float rms2db(float value)
{
  return 20 * log10(value);
}

void MainWindow::rmscallback(float* values, size_t size, void* user_data)
{
  
 // VAGG_LOG(VAGG_LOG_DEBUG, "size %Zu  values %f",size, values[0]);
  
  MainWindow* mw = static_cast<MainWindow*>(user_data);
  for (size_t i = 0; i < size; i++) {
// VAGG_LOG(VAGG_LOG_DEBUG, "rmscb size %Zu  i %Zu  val %f",size, i, values[i]);
    values[i] = rms2db(values[i]);
  }
  mw->rmscallback_m(values, size, user_data);
}

void MainWindow::rmscallback_m(float* values, size_t size, void* user_data)
{
  dbm->valueChanged(values, size);
}

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
  QString file = QFileDialog::getOpenFileName(this, tr("Select a .wav audio file."),
      QDesktopServices::storageLocation(QDesktopServices::MusicLocation),
      tr("Wav file (*.wav)"));

  filepath = file;
  player = new AudioPlayer(4096);
  player->insert(new RMS(&MainWindow::rmscallback, this));

  QByteArray ba = filepath.toLocal8Bit();
  const char *c_str = ba.data();
  player->load(c_str);
  playAction->setDisabled(false);

  filepath = file;
  filenameLabel->setText(file);
  

  seekSlider->setDisabled(false);
  volumeSlider->setDisabled(false);
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
  if (player) {
    player->play();
    event_loop_timer.start(1);
    playing = true;
    playAction->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
  }
}

void MainWindow::pause()
{
  if (player) {
    event_loop_timer.stop();
    player->pause();
    playing = false;
    playAction->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
  }
}

void MainWindow::seek(int seek)
{
  if (player && !current_time_advance_) {
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
  if (player) {
    player->unload();
    playAction->setDisabled(true);
    filepath.clear();
    stopped();
    seekSlider->setDisabled(true);
    volumeSlider->setDisabled(true);
  }
}

void MainWindow::unload()
{
  playing = false;
  if (player) {
    delete player;
  }
  player = 0;
  stopped();
}

void MainWindow::stopped()
{
  playing = false;
  playAction->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
  seekSlider->setDisabled(true);
  volumeSlider->setDisabled(true);
}

void MainWindow::event_loop()
{
  current_time_advance_ = true;
  double current_time = player->current_time();
  int pos = current_time / player->duration() * seekSlider->maximum();
  seekSlider->setValue(pos);
  int seconds = static_cast<int>(current_time) % 60;
  int minutes = current_time / 60;
  QString text(QString::number(minutes) + " " + QString::number(seconds));
  timeLcd->display(text);
  current_time_advance_ = false;
  if (player && ! player->state_machine()) {
    event_loop_timer.stop();
    stop();
  }
}

void MainWindow::about()
{
  QMessageBox::information(this, tr("About Music Player"),
      tr("This player is a demo for DT2410 Lab 2011"));
}

void MainWindow::setupActions()
{
  openAction = new QAction(style()->standardIcon(QStyle::SP_ArrowUp), tr("Test"), this);
  openAction->setDisabled(false);
  playAction = new QAction(style()->standardIcon(QStyle::SP_MediaPlay), tr("Play"), this);
  playAction->setShortcut(tr("Ctrl+P"));
  playAction->setDisabled(true);
  nextAction = new QAction(style()->standardIcon(QStyle::SP_MediaSkipForward), tr("Next"), this);
  nextAction->setShortcut(tr("Ctrl+N"));
  previousAction = new QAction(style()->standardIcon(QStyle::SP_MediaSkipBackward), tr("Previous"), this);
  previousAction->setShortcut(tr("Ctrl+R"));
  addFilesAction = new QAction(tr("Open &File"), this);
  addFilesAction->setShortcut(tr("Ctrl+F"));
  exitAction = new QAction(tr("E&xit"), this);
  exitAction->setShortcuts(QKeySequence::Quit);
  aboutAction = new QAction(tr("A&bout"), this);
  aboutAction->setShortcut(tr("Ctrl+B"));
  aboutQtAction = new QAction(tr("About Qt"), this);

  connect(playAction, SIGNAL(triggered()), this, SLOT(playpause()));

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
  bar->addAction(openAction);


  seekSlider = new QSlider(Qt::Horizontal, this);
  seekSlider->setFocusPolicy(Qt::StrongFocus);
  seekSlider->setTickPosition(QSlider::NoTicks);
  seekSlider->setTickInterval(10);
  seekSlider->setSingleStep(1);
  seekSlider->setRange(0,1000);
  seekSlider->setDisabled(true);

  dbm = new dBMeter(this);


  volumeSlider = new QSlider(Qt::Horizontal, this);
  volumeSlider->setFocusPolicy(Qt::StrongFocus);
  volumeSlider->setTickPosition(QSlider::TicksBothSides);
  volumeSlider->setTickInterval(10);
  //volumeSlider->setSingleStep(1);
  volumeSlider->setRange(0,100);
  volumeSlider->setDisabled(true);
  

  QLabel *volumeLabel = new QLabel;
  volumeLabel->setPixmap(QPixmap("images/volume.png"));

  QPalette palette;
  palette.setBrush(QPalette::Light, Qt::darkGray);

  timeLcd = new QLCDNumber;
  timeLcd->setPalette(palette);
  
  filenameLabel = new QLabel("load file!");


  /*QStringList headers;
  headers << tr("Title");
  */

  QHBoxLayout *dbmLayout = new QHBoxLayout;
  dbmLayout->addWidget(dbm);

  connect(volumeSlider, SIGNAL(valueChanged(int)), this, SLOT(set_volume(int)));

  QHBoxLayout *seekerLayout = new QHBoxLayout;
  seekerLayout->addWidget(seekSlider);
  seekerLayout->addWidget(timeLcd);

  QHBoxLayout *playbackLayout = new QHBoxLayout;
  playbackLayout->addWidget(bar);
  playbackLayout->addStretch();
  playbackLayout->addWidget(volumeLabel);
  playbackLayout->addWidget(volumeSlider);

  QHBoxLayout *mid = new QHBoxLayout;
  mid->addWidget(filenameLabel);
  mid->addLayout(dbmLayout);


  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addLayout(mid);
  mainLayout->addLayout(seekerLayout);
  mainLayout->addLayout(playbackLayout);

  QWidget *widget = new QWidget;
  widget->setLayout(mainLayout);

  setCentralWidget(widget);
  setWindowTitle("wav player 0.1");
}

void MainWindow::set_volume(int val)
{
	float vol = (float)val/100;
	//VAGG_LOG(VAGG_LOG_DEBUG, "val %d -- vol %f",val, vol);
	player->set_volume(vol);
}
