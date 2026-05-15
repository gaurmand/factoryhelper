#include "mainwindow.h"

#include <factory/model/model.h>

namespace chess
{

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
   setWindowTitle(tr("factoryhelper"));
   setMinimumSize(QSize(600, 400));

   factory::Model m;
   m.test();
}

} // namespace chess
