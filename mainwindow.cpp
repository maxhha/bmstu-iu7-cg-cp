#include "mainwindow.h"
#include "src/Polygon.h"
#include "src/QtEngine.h"
#include "ui_mainwindow.h"
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QPolygonF>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    CGCP::Polygon p({
        CGCP::Point(0, 0, 0),
        CGCP::Point(100, 0, 0),
        CGCP::Point(0, 100, 0),
    });

    auto view = ui->graphicsView;
    engine_ = std::make_unique<QtEngine>(view);
    engine_->drawer().get("main");
}

MainWindow::~MainWindow()
{
    delete ui;
}
