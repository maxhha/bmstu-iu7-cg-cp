#include "mainwindow.h"
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

    // std::unique_ptr<CGCP::ContinuesFunction>
    //     ff = std::make_unique<CGCP::FieldFunction>(f, domain);

    // engine_->loader()
    //     .get("raw")
    //     .load(
    //         "blabla",
    //         [=](CGCP::Error err, std::unique_ptr<CGCP::ContinuesFunction> ff, double progress) -> void
    //         {
    //             qDebug() << "load" << err << progress;

    //             if (ff)
    //             {
    //                 engine_->polygonizer()
    //                     .get("dmc")
    //                     .function(ff);

    //                 engine_->polygonizer()
    //                     .get("dmc")
    //                     .run(
    //                         [=](std::shared_ptr<CGCP::Mesh> mesh, double percent) -> void
    //                         {
    //                             emit polygonizer_progress(mesh, percent);
    //                         });
    //             }
    //         });
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

    if (percent >= 1 - std::numeric_limits<double>::epsilon())
    {
        polygonizer_dialog_->close();
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
    // QString filename("C:/Users/devma/Projects/bmstu-iu7-cg-cp/data/TeddyBear/Teddybear.dat");
    QString filename = QFileDialog::getOpenFileName(this);

    engine_->loader()
        .get("raw")
        .load(
            filename.toStdString(),
            [&](CGCP::Error err, std::unique_ptr<CGCP::ContinuesFunction> f, double percent) -> void
            {
                emit loader_progress(err, std::move(f), percent);
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

    // engine_->polygonizer()
    //     .get("dmc")
    //     .run(
    //         [&](std::shared_ptr<CGCP::Mesh> mesh, double percent) -> void
    //         {
    //             emit polygonizer_progress(mesh, percent);
    //         });

    // polygonizer_dialog_->reset();
    // polygonizer_dialog_->show();

    return;
}

void MainWindow::handle_loader_finish(FunctionPtr f)
{
    QString shape_x, shape_y, shape_z;
    double scale_x, scale_y, scale_z;

    if (f)
    {
        engine_->polygonizer()
            .get("dmc")
            .function(f);

        auto ti_f = std::dynamic_pointer_cast<CGCP::TIFunction>(f);

        if (!ti_f)
        {
            qDebug() << "unknown function type";
        }
        else
        {
            shape_x = QString::number(ti_f->scan().shape().x());
            shape_y = QString::number(ti_f->scan().shape().y());
            shape_z = QString::number(ti_f->scan().shape().z());

            scale_x = ti_f->scan().scale().x();
            scale_y = ti_f->scan().scale().y();
            scale_z = ti_f->scan().scale().z();
        }
    }

    this->ui->labelShapeX->setText(shape_x);
    this->ui->labelShapeY->setText(shape_y);
    this->ui->labelShapeZ->setText(shape_z);

    this->ui->labelVoxelX->setValue(scale_x);
    this->ui->labelVoxelY->setValue(scale_y);
    this->ui->labelVoxelZ->setValue(scale_z);

    // this->ui->labelShapeX->update();
    // this->ui->labelShapeY->update();
    // this->ui->labelShapeZ->update();

    // this->ui->labelVoxelX->update();
    // this->ui->labelVoxelY->update();
    // this->ui->labelVoxelZ->update();

    QCoreApplication::processEvents();

    // this->ui->load->update();
}