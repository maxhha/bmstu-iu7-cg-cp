#include "mainwindow.h"
#include "src/FieldFunction.h"
#include "src/QtEngine.h"
#include "src/QtMeshDrawer.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMetaType>
#include <QPolygonF>
#include <QResizeEvent>
#include <math.h>
#include <type_traits>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    qRegisterMetaType<std::shared_ptr<CGCP::Mesh>>("std::shared_ptr<CGCP::Mesh>");

    QObject::connect(
        this,
        &MainWindow::polygonizer_progress,
        this,
        &MainWindow::handle_polygonizer_progress);

    auto view = ui->graphicsView;
    engine_ = std::make_unique<QtEngine>(view);

    dialog_ = new QProgressDialog("Полигонизация...", "Отменить", 0, 100, this);
    dialog_->setWindowModality(Qt::WindowModal);

    QObject::connect(
        dialog_,
        &QProgressDialog::canceled,
        this,
        &MainWindow::handle_cancel_polygonizer);

    dialog_->show();

    auto f = [](double x, double y, double z) -> double
    {
        return x * x + y * y - z;
    };

    CGCP::AABB domain(-10, -10, -10, 10, 10, 10);

    // std::unique_ptr<CGCP::ContinuesFunction>
    //     ff = std::make_unique<CGCP::FieldFunction>(f, domain);

    engine_->loader()
        .get("raw")
        .load(
            "blabla",
            [=](CGCP::Error err, std::unique_ptr<CGCP::ContinuesFunction> ff, double progress) -> void
            {
                qDebug() << "load" << err << progress;

                if (ff)
                {
                    engine_->polygonizer()
                        .get("dmc")
                        .function(ff);

                    engine_->polygonizer()
                        .get("dmc")
                        .run(
                            [=](std::shared_ptr<CGCP::Mesh> mesh, double percent) -> void
                            {
                                emit polygonizer_progress(mesh, percent);
                            });
                }
            });
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::handle_polygonizer_progress(std::shared_ptr<CGCP::Mesh> mesh, double percent)
{
    if (dialog_->wasCanceled())
    {
        return;
    }

    dialog_->setValue(percent * 100);

    if (percent >= 1 - std::numeric_limits<double>::epsilon())
    {
        dialog_->close();
    }

    if (mesh)
    {
        qDebug() << "triangles:" << mesh->triangles().size();
        engine_->drawer().get("main").setMesh(mesh);
    }
}

void MainWindow::handle_cancel_polygonizer()
{
    engine_->polygonizer().get("dmc").cancel();
}

void MainWindow::on_buttonTranslate_clicked()
{
    double x = ui->inputTranslateX->value();
    double y = ui->inputTranslateY->value();
    double z = ui->inputTranslateZ->value();

    engine_->drawer().get("main").translate(CGCP::Vec3Df(x, y, -z));
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