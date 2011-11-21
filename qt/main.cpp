 #include <QtGui>

 #include "mainwindow.h"
 #include "dbmeter.h"

 int main(int argv, char **args)
 {
     qsrand(time(0));
     QApplication app(argv, args);
     app.setApplicationName("DT2410 Lab");
     app.setQuitOnLastWindowClosed(true);


	
     MainWindow window;
     window.show();
     
	 //dBMeter dbm(&window);


     return app.exec();
 }
