#include "mainwindow.h"
#include "src/Polygon.h"
#include "src/QtEngine.h"
#include "src/QtMeshDrawer.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QPolygonF>
#include <QResizeEvent>
#include <math.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    CGCP::Vec3Df v0(-5, -5, -5);
    CGCP::Vec3Df v1(-5, -5, +5);
    CGCP::Vec3Df v2(-5, +5, -5);
    CGCP::Vec3Df v3(-5, +5, +5);
    CGCP::Vec3Df v4(+5, -5, -5);
    CGCP::Vec3Df v5(+5, -5, +5);
    CGCP::Vec3Df v6(+5, +5, -5);
    CGCP::Vec3Df v7(+5, +5, +5);

    auto mesh = std::make_shared<CGCP::Mesh>(CGCP::Mesh({
        CGCP::Triangle3Df(v0, v1, v3),
        CGCP::Triangle3Df(v0, v2, v3),
        CGCP::Triangle3Df(v4, v5, v7),
        CGCP::Triangle3Df(v4, v6, v7),

        CGCP::Triangle3Df(v0, v1, v5),
        CGCP::Triangle3Df(v0, v4, v5),
        CGCP::Triangle3Df(v2, v3, v7),
        CGCP::Triangle3Df(v2, v6, v7),

        CGCP::Triangle3Df(v1, v3, v7),
        CGCP::Triangle3Df(v1, v5, v7),
        CGCP::Triangle3Df(v0, v4, v6),
        CGCP::Triangle3Df(v0, v2, v6),

    }));

    mesh->origin() = CGCP::Vec3Df(0, 0, 0);

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

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QApplication::sendEvent(dynamic_cast<QtMeshDrawer *>(&engine_->drawer().get("main")), event);
}