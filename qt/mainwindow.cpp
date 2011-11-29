 #include <QtGui>

#include <stdio.h>
#include <sndfile.h>
#include <string.h>
#include <vector>
#include "vagg/vagg_macros.h"
#include <portaudio.h>

 #include "mainwindow.h"
 
 using namespace std;

const char* FILENAME = "../assets/amen.wav";
const size_t CHUNK_SIZE = 2*4096;
const size_t CHANNELS = 2;
const unsigned SAMPLERATE = 44100;
const size_t FRAMES_PER_BUFFER = 4096;

typedef float SamplesType ;
typedef vector<SamplesType> AudioBuffer;

struct PlaybackStatus {
  size_t index;
  AudioBuffer& samples;

  PlaybackStatus(AudioBuffer& samples)
  :index(0),samples(samples)
  { }
};

 

 MainWindow::MainWindow()
 {
  
  	 /*      
     audioOutput = new Phonon::AudioOutput(Phonon::MusicCategory, this);
     mediaObject = new Phonon::MediaObject(this);
     metaInformationResolver = new Phonon::MediaObject(this);
	


     mediaObject->setTickInterval(1000);
     connect(mediaObject, SIGNAL(tick(qint64)), this, SLOT(tick(qint64)));
     connect(mediaObject, SIGNAL(stateChanged(Phonon::State,Phonon::State)), this, SLOT(stateChanged(Phonon::State,Phonon::State)));
     connect(metaInformationResolver, SIGNAL(stateChanged(Phonon::State,Phonon::State)), this, SLOT(metaStateChanged(Phonon::State,Phonon::State)));
     connect(mediaObject, SIGNAL(currentSourceChanged(Phonon::MediaSource)), this, SLOT(sourceChanged(Phonon::MediaSource)));
     connect(mediaObject, SIGNAL(aboutToFinish()), this, SLOT(aboutToFinish()));

     Phonon::createPath(mediaObject, audioOutput);
	*/
	
	
     setupActions();
     setupMenus();
     setupUi();
     timeLcd->display("00:00");
 }

 void MainWindow::addFiles()
 {
     QStringList files = QFileDialog::getOpenFileNames(this, tr("Select Music Files"),
         QDesktopServices::storageLocation(QDesktopServices::MusicLocation));

     if (files.isEmpty())
         return;

     int index = sources.size();
     foreach (QString string, files) {
             Phonon::MediaSource source(string);

         sources.append(source);
     }
     if (!sources.isEmpty())
         metaInformationResolver->setCurrentSource(sources.at(index));

 }
 

void read_file(const char* filename, AudioBuffer& samples)
{
  SF_INFO infos_read;
  infos_read.samplerate = SAMPLERATE;
  infos_read.channels = CHANNELS;
  infos_read.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;

  VAGG_SYSCALL(sf_format_check(&infos_read));

  SNDFILE *file = sf_open(filename, SFM_READ, &infos_read);
  if (file == NULL) {
    VAGG_LOG(VAGG_LOG_FATAL, "%s", sf_strerror(file));
    abort();
  }

  size_t count = 0;
  do {
    SamplesType tmp[CHUNK_SIZE*CHANNELS];
    count = sf_read_float(file, tmp, CHUNK_SIZE*CHANNELS);
    samples.insert(samples.end(), tmp, tmp + count);
  } while (count == CHUNK_SIZE*CHANNELS);

  if (sf_close(file) != 0) {
    VAGG_LOG(VAGG_LOG_OK, "Error while closing the file.");
  }
}


 
 void MainWindow::dtest(){
	//QMessageBox::information(this, tr("Test Button!"),tr("You pressed it."));
   	timeLcd->display("23:42");
   	dbm->newval(qrand()%200);
   	AudioBuffer ab;
   	read_file("../assets/amen.wav",ab);
   	
 }




 void MainWindow::about()
 {
     QMessageBox::information(this, tr("About Music Player"),
         tr("This player is a demo for DT2410 Lab"));
 }

 void MainWindow::stateChanged(Phonon::State newState, Phonon::State /* oldState */)
 {
     switch (newState) {
         case Phonon::ErrorState:
         /*
             if (mediaObject->errorType() == Phonon::FatalError) {
                 QMessageBox::warning(this, tr("Fatal Error"),
                 mediaObject->errorString());
             } else {
                 QMessageBox::warning(this, tr("Error"),
                 mediaObject->errorString());
             }
             */
             
             break;
         case Phonon::PlayingState:
                 playAction->setEnabled(false);
                 pauseAction->setEnabled(true);
                 stopAction->setEnabled(true);
                 break;
         case Phonon::StoppedState:
                 stopAction->setEnabled(false);
                 playAction->setEnabled(true);
                 pauseAction->setEnabled(false);
                 timeLcd->display("00:00");
                 break;
         case Phonon::PausedState:
                 pauseAction->setEnabled(false);
                 stopAction->setEnabled(true);
                 playAction->setEnabled(true);
                 break;
         case Phonon::BufferingState:
                 break;
         default:
             ;
     }
 }

 void MainWindow::tick(qint64 time)
 {
     QTime displayTime(0, (time / 60000) % 60, (time / 1000) % 60);

     timeLcd->display(displayTime.toString("mm:ss"));
 }

 void MainWindow::tableClicked(int row, int /* column */)
 {
     /*bool wasPlaying = mediaObject->state() == Phonon::PlayingState;

     mediaObject->stop();
     mediaObject->clearQueue();

     if (row >= sources.size())
         return;

     mediaObject->setCurrentSource(sources[row]);

     if (wasPlaying)
         mediaObject->play();
     else
         mediaObject->stop();*/
 }

 void MainWindow::sourceChanged(const Phonon::MediaSource &source)
 {
     musicTable->selectRow(sources.indexOf(source));
     timeLcd->display("00:00");
 }

 void MainWindow::metaStateChanged(Phonon::State newState, Phonon::State /* oldState */)
 {
     if (newState == Phonon::ErrorState) {
         QMessageBox::warning(this, tr("Error opening files"),
             metaInformationResolver->errorString());
         while (!sources.isEmpty() &&
                !(sources.takeLast() == metaInformationResolver->currentSource())) {}  /* loop */;
         return;
     }

     if (newState != Phonon::StoppedState && newState != Phonon::PausedState)
         return;

     if (metaInformationResolver->currentSource().type() == Phonon::MediaSource::Invalid)
             return;

     QMap<QString, QString> metaData = metaInformationResolver->metaData();

     QString title = metaData.value("TITLE");
     if (title == "")
         title = metaInformationResolver->currentSource().fileName();

	
     QTableWidgetItem *titleItem = new QTableWidgetItem(title);
     titleItem->setFlags(titleItem->flags() ^ Qt::ItemIsEditable);
     /*
     QTableWidgetItem *artistItem = new QTableWidgetItem(metaData.value("ARTIST"));
     artistItem->setFlags(artistItem->flags() ^ Qt::ItemIsEditable);
     QTableWidgetItem *albumItem = new QTableWidgetItem(metaData.value("ALBUM"));
     albumItem->setFlags(albumItem->flags() ^ Qt::ItemIsEditable);
     QTableWidgetItem *yearItem = new QTableWidgetItem(metaData.value("DATE"));
     yearItem->setFlags(yearItem->flags() ^ Qt::ItemIsEditable);

	*/

     int currentRow = musicTable->rowCount();
     musicTable->insertRow(currentRow);
     musicTable->setItem(currentRow, 0, titleItem);
     /*
     musicTable->setItem(currentRow, 1, artistItem);
     musicTable->setItem(currentRow, 2, albumItem);
     musicTable->setItem(currentRow, 3, yearItem);
     */

     if (musicTable->selectedItems().isEmpty()) {
         musicTable->selectRow(0);
        // mediaObject->setCurrentSource(metaInformationResolver->currentSource());
     }

     Phonon::MediaSource source = metaInformationResolver->currentSource();
     int index = sources.indexOf(metaInformationResolver->currentSource()) + 1;
     if (sources.size() > index) {
         metaInformationResolver->setCurrentSource(sources.at(index));
     }
     else {
         musicTable->resizeColumnsToContents();
         if (musicTable->columnWidth(0) > 300)
             musicTable->setColumnWidth(0, 300);
     }
 }

 void MainWindow::aboutToFinish()
 {
     /*int index = sources.indexOf(mediaObject->currentSource()) + 1;
     if (sources.size() > index) {
         mediaObject->enqueue(sources.at(index));
     }*/
 }

 void MainWindow::setupActions()
 {
     testAction = new QAction(style()->standardIcon(QStyle::SP_ArrowUp), tr("Test"), this);
     testAction->setDisabled(false);
     playAction = new QAction(style()->standardIcon(QStyle::SP_MediaPlay), tr("Play"), this);
     playAction->setShortcut(tr("Ctrl+P"));
     playAction->setDisabled(true);
     pauseAction = new QAction(style()->standardIcon(QStyle::SP_MediaPause), tr("Pause"), this);
     pauseAction->setShortcut(tr("Ctrl+A"));
     pauseAction->setDisabled(true);
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

    /* connect(playAction, SIGNAL(triggered()), mediaObject, SLOT(play()));
     connect(pauseAction, SIGNAL(triggered()), mediaObject, SLOT(pause()) );
     connect(stopAction, SIGNAL(triggered()), mediaObject, SLOT(stop()));
     
     */
     
     connect(addFilesAction, SIGNAL(triggered()), this, SLOT(addFiles()));
     connect(exitAction, SIGNAL(triggered()), this, SLOT(close()));
     connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));
     connect(aboutQtAction, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
	 connect(testAction, SIGNAL(triggered()), this, SLOT(dtest()));

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
     bar->addAction(pauseAction);
     bar->addAction(stopAction);
     bar->addAction(testAction);

/*
     seekSlider = new Phonon::SeekSlider(this);
     seekSlider->setMediaObject(mediaObject);
  */   
     dbm = new dBMeter(this);
//     connect(testAction, SIGNAL(triggered()), dbm, SLOT(newval(55)));
  //  connect(testAction, SIGNAL(triggered()), dbm, SLOT(newval(int)));
/*
     volumeSlider = new Phonon::VolumeSlider(this);
     volumeSlider->setAudioOutput(audioOutput);
     volumeSlider->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
*/
//     QLabel *volumeLabel = new QLabel;
//     volumeLabel->setPixmap(QPixmap("images/volume.png"));

     QPalette palette;
     palette.setBrush(QPalette::Light, Qt::darkGray);

     timeLcd = new QLCDNumber;
     timeLcd->setPalette(palette);
     
     QStringList headers;
     headers << tr("Title");

     musicTable = new QTableWidget(0, 1);
     musicTable->setHorizontalHeaderLabels(headers);
     musicTable->setSelectionMode(QAbstractItemView::SingleSelection);
     musicTable->setSelectionBehavior(QAbstractItemView::SelectRows);
     connect(musicTable, SIGNAL(cellPressed(int,int)), this, SLOT(tableClicked(int,int)));
     
     QHBoxLayout *dbmLayout = new QHBoxLayout;
     dbmLayout->addWidget(dbm);
             

     QHBoxLayout *seekerLayout = new QHBoxLayout;
     //seekerLayout->addWidget(seekSlider);
     seekerLayout->addWidget(timeLcd);

	
     QHBoxLayout *playbackLayout = new QHBoxLayout;
     playbackLayout->addWidget(bar);
     //playbackLayout->addStretch();
    //playbackLayout->addWidget(volumeLabel);
    // playbackLayout->addWidget(volumeSlider);
    
     QHBoxLayout *mid = new QHBoxLayout;
     mid->addWidget(musicTable);
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
