#include "mainwindow.h"
#include "src/Exception.h"
#include "src/FieldFunction.h"
#include "src/QtEngine.h"
#include "src/QtMeshDrawer.h"
#include "src/TIFunction.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QFileDialog>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMessageBox>
#include <QMetaType>
#include <QPolygonF>
#include <QResizeEvent>
#include <math.h>
#include <type_traits>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    qRegisterMetaType<MeshPtr>("MeshPtr");
    qRegisterMetaType<FunctionPtr>("FunctionPtr");
    qRegisterMetaType<CGCP::Error>("CGCP::Error");

    QObject::connect(
        this,
        &MainWindow::polygonizer_progress,
        this,
        &MainWindow::handle_polygonizer_progress);

    QObject::connect(
        this,
        &MainWindow::loader_progress,
        this,
        &MainWindow::handle_loader_progress);

    auto view = ui->graphicsView;
    engine_ = std::make_unique<QtEngine>(view);

    polygonizer_dialog_ = new QProgressDialog("Полигонизация...", "Отменить", 0, 100, this);
    polygonizer_dialog_->setWindowModality(Qt::WindowModal);
    polygonizer_dialog_->close();

    loader_dialog_ = new QProgressDialog("Загрузка...", "Отменить", 0, 100, this);
    loader_dialog_->setWindowModality(Qt::WindowModal);
    loader_dialog_->close();

    QObject::connect(
        polygonizer_dialog_,
        &QProgressDialog::canceled,
        this,
        &MainWindow::handle_cancel_polygonizer);

    QObject::connect(
        loader_dialog_,
        &QProgressDialog::canceled,
        this,
        &MainWindow::handle_cancel_loader);
}

MainWindow::~MainWindow()
{
    delete loader_dialog_;
    delete polygonizer_dialog_;
    delete ui;
}

void MainWindow::handle_polygonizer_progress(std::shared_ptr<CGCP::Mesh> mesh, double percent)
{
    if (polygonizer_dialog_->wasCanceled())
    {
        return;
    }

    polygonizer_dialog_->setValue(percent * 100);

    if (mesh)
    {
        polygonizer_dialog_->close();
        ui->labelTriangles->setText(QString::number(mesh->triangles().size()));
        engine_->drawer().get("main").setMesh(mesh);
    }
}

void MainWindow::handle_cancel_polygonizer()
{
    engine_->polygonizer().get("dmc").cancel();
}

void MainWindow::handle_cancel_loader()
{
    engine_->loader().get("raw").cancel();
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

void MainWindow::on_buttonOpen_clicked()
{
    // QString filename("C:/Users/devma/Projects/bmstu-iu7-cg-cp/data/Sphere/Sphere.dat");
    QString filename = QFileDialog::getOpenFileName(this);

    engine_->loader()
        .get("raw")
        .load(
            filename.toStdString(),
            [&](CGCP::Error err, std::shared_ptr<CGCP::TomographyScan> scan, double percent) -> void
            {
                emit loader_progress(
                    err,
                    scan ? std::make_shared<CGCP::TIFunction>(scan) : nullptr,
                    percent);
            });

    loader_dialog_->reset();
    loader_dialog_->show();
}

const char *messageForError(CGCP::Error err)
{
    switch (err)
    {
    case CGCP::Error::OK:
        return "Нет ошибки!";
    case CGCP::Error::NO_FILE:
        return "Не удалось открыть файл.";
    default:
        return "Неизвестная ошибка";
    }
}

void MainWindow::handle_loader_progress(
    CGCP::Error err,
    std::shared_ptr<CGCP::ContinuesFunction> f,
    double percent)
{
    if (loader_dialog_->wasCanceled())
    {
        return;
    }

    if (err != CGCP::Error::OK)
    {
        loader_dialog_->close();
        QMessageBox msg;
        msg.setText(messageForError(err));
        msg.exec();

        return;
    }

    if (!f)
    {
        loader_dialog_->setValue(percent * 100);
        return;
    }

    handle_loader_finish(f);

    loader_dialog_->close();
}

void MainWindow::handle_loader_finish(FunctionPtr f)
{
    QString shape_x = "", shape_y = "", shape_z = "";
    double scale_x = 0, scale_y = 0, scale_z = 0;

    if (f)
    {
        engine_->function().set(f);

        auto ti_f = std::dynamic_pointer_cast<CGCP::TIFunction>(f);

        if (!ti_f)
        {
            qDebug() << "unknown function type";
        }
        else
        {
            shape_x = QString::number(ti_f->scan()->shape().x());
            shape_y = QString::number(ti_f->scan()->shape().y());
            shape_z = QString::number(ti_f->scan()->shape().z());

            scale_x = ti_f->scan()->scale().x();
            scale_y = ti_f->scan()->scale().y();
            scale_z = ti_f->scan()->scale().z();
        }
    }

    ui->labelShapeX->setText(shape_x);
    ui->labelShapeY->setText(shape_y);
    ui->labelShapeZ->setText(shape_z);

    ui->labelVoxelX->setValue(scale_x);
    ui->labelVoxelY->setValue(scale_y);
    ui->labelVoxelZ->setValue(scale_z);

    QCoreApplication::processEvents();
}

void MainWindow::on_buttonPolygonize_clicked()
{
    auto f = compose_function();
    engine_->polygonizer().get("dmc").function(f);

    auto config = engine_->polygonizer().get("dmc").config();

    config["grid_dim_x"] = QString::number(ui->inputDimX->value()).toStdString();
    config["grid_dim_y"] = QString::number(ui->inputDimY->value()).toStdString();
    config["grid_dim_z"] = QString::number(ui->inputDimZ->value()).toStdString();

    config["tolerance"] = QString::number(ui->inputTolerance->value()).toStdString();
    config["max_depth"] = QString::number(ui->inputMaxDepth->value()).toStdString();

    try
    {
        engine_->polygonizer().get("dmc").config(config);

        engine_->polygonizer()
            .get("dmc")
            .run(
                [&](std::shared_ptr<CGCP::Mesh> mesh, double percent) -> void
                {
                    emit polygonizer_progress(mesh, percent);
                });
    }
    catch (CGCP::Exception &e)
    {
        const char *text = e.message().c_str();

        if (e.message().compare("Function is not set") == 0)
        {
            text = "Нет данных томографии";
        }

        QMessageBox msg;
        msg.setText(text);
        msg.exec();

        return;
    }

    polygonizer_dialog_->reset();
    polygonizer_dialog_->show();
}

void MainWindow::on_buttonRemoveMesh_clicked()
{
    engine_->drawer().get("main").setMesh(nullptr);
}

void MainWindow::on_buttonChangeVoxel_clicked()
{
    auto f = engine_->function().result();

    if (!f)
    {
        QMessageBox msg;
        msg.setText("Сначала загрузите файл");
        msg.exec();
    }

    auto nn_f = std::dynamic_pointer_cast<CGCP::TIFunction>(f);

    if (!nn_f)
    {
        qDebug() << "unknown function type";
    }
    else
    {
        auto scan = nn_f->scan();
        scan->scale().x() = ui->labelVoxelX->value();
        scan->scale().y() = ui->labelVoxelY->value();
        scan->scale().z() = ui->labelVoxelZ->value();

        engine_->function().set(std::make_shared<CGCP::TIFunction>(scan));
    }
}

MainWindow::FunctionPtr MainWindow::compose_function()
{
    return engine_->function()
        .add(
            "average",
            {
                {"size",
                 QString::number(ui->inputAverage->value()).toStdString()},
                {"step_x",
                 QString::number(ui->labelVoxelX->value()).toStdString()},
                {"step_y",
                 QString::number(ui->labelVoxelY->value()).toStdString()},
                {"step_z",
                 QString::number(ui->labelVoxelZ->value()).toStdString()},
            })
        .add(
            "offset",
            {
                {"value",
                 QString::number(-ui->inputOffset->value()).toStdString()},
            })
        .result();
}

void MainWindow::on_buttonReset_clicked()
{
    engine_->drawer().get("main").resetTransformation();
}
