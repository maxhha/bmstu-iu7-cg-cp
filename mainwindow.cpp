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

    auto mesh = std::make_shared<CGCP::Mesh>(CGCP::Mesh({CGCP::Triangle3Df(CGCP::Vec3Df(0, 0, 0),
                                                                           CGCP::Vec3Df(100, 0, 0),
                                                                           CGCP::Vec3Df(0, 100, 0))}));

    CGCP::Polygon p({
        CGCP::Point(0, 0, 0),
        CGCP::Point(100, 0, 0),
        CGCP::Point(0, 100, 0),
    });

    auto view = ui->graphicsView;
    engine_ = std::make_unique<QtEngine>(view);
    engine_->drawer().get("main").setMesh(mesh);
}

MainWindow::~MainWindow()
{
    delete ui;
}
