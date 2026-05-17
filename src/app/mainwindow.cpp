#include "mainwindow.h"

#include <factory/model/productiongraph.h>

namespace chess
{

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
   setWindowTitle(tr("factoryhelper"));
   setMinimumSize(QSize(600, 400));

   factory::ProductionGraph g;
   [[maybe_unused]] auto items = g.vertices<factory::ProductionGraph::Item>();
   [[maybe_unused]] auto recipes =
      g.vertices<factory::ProductionGraph::Recipe>();
}

} // namespace chess
