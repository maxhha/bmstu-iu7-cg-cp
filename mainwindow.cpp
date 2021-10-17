#include "mainwindow.h"
#include "src/QtEngine.h"
#include "src/QtMeshDrawer.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QPolygonF>
#include <QResizeEvent>
#include <math.h>
#include <type_traits>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QObject::connect(
        this,
        &MainWindow::polygonizer_progress,
        this,
        &MainWindow::handle_polygonizer_progress);

    auto view = ui->graphicsView;
    engine_ = std::make_unique<QtEngine>(view);

    dialog_ = new QProgressDialog("Полигонизация...", "Отменить", 0, 100, this);
    dialog_->setWindowModality(Qt::WindowModal);

    dialog_->show();

    engine_->polygonizer()
        .get("dmc")
        .run(
            [=](std::shared_ptr<CGCP::Mesh> mesh, double percent) -> void
            {
                if (dialog_->wasCanceled())
                {
                    engine_->polygonizer().get("dmc").cancel();
                    return;
                }

                emit polygonizer_progress(percent);

                if (mesh)
                {
                    engine_->drawer().get("main").setMesh(mesh);
                }
            });
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::handle_polygonizer_progress(double percent)
{
    dialog_->setValue(percent * 100);

    if (percent >= 1 - std::numeric_limits<double>::epsilon())
    {
        dialog_->close();
    }
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