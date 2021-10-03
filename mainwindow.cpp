#include "mainwindow.h"
#include "src/Polygon.h"
#include "src/QtEngine.h"
#include "ui_mainwindow.h"
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QPolygonF>
#include <math.h>

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

void MainWindow::on_buttonTranslate_clicked()
{
    double x = ui->inputTranslateX->value();
    double y = ui->inputTranslateY->value();
    double z = ui->inputTranslateZ->value();

    engine_->drawer().get("main").translate(CGCP::Vec3Df(x, y, z));
}

void MainWindow::on_buttonScale_clicked()
{
    double x = ui->inputScaleX->value();
    double y = ui->inputScaleY->value();
    double z = ui->inputScaleZ->value();

    engine_->drawer().get("main").scale(CGCP::Vec3Df(x, y, z));
}

void MainWindow::on_buttonRotate_clicked()
{
    double x = ui->inputRotateX->value();
    double y = ui->inputRotateY->value();
    double z = ui->inputRotateZ->value();
    double angle = ui->inputRotateAngle->value();

    engine_->drawer().get("main").rotate(CGCP::Vec3Df(x, y, z), angle);
}
