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
#include <fmt/format.h>
#include <math.h>
#include <type_traits>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    qRegisterMetaType<MeshPtr>("MeshPtr");
    qRegisterMetaType<ScanPtr>("ScanPtr");
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

    QObject::connect(
        this,
        &MainWindow::preprocess_progress,
        this,
        &MainWindow::handle_preprocess_progress);

    auto view = ui->graphicsView;
    engine_ = std::make_unique<QtEngine>(view);

    preprocess_dialog_ = new QProgressDialog("Подготовка томографии...", "Отменить", 0, 100, this);
    preprocess_dialog_->setWindowModality(Qt::WindowModal);
    preprocess_dialog_->close();

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
        preprocess_dialog_,
        &QProgressDialog::canceled,
        this,
        &MainWindow::handle_cancel_preprocess);

    QObject::connect(
        loader_dialog_,
        &QProgressDialog::canceled,
        this,
        &MainWindow::handle_cancel_loader);

    ui->inputColor1->setColor(QColor("#888088"));
    ui->inputColor2->setColor(QColor("#200036"));
    ui->inputColor3->setColor(QColor("#200036"));
    ui->inputColor4->setColor(QColor("#100018"));
    ui->inputColorBorder->setColor(QColor("#ff3333"));

    triangles_label_ = new QLabel("Разрешение сцены:");
    ui->statusbar->addPermanentWidget(triangles_label_);
    triangles_label_ = new QLabel();
    ui->statusbar->addPermanentWidget(triangles_label_);
}

MainWindow::~MainWindow()
{
    delete loader_dialog_;
    delete polygonizer_dialog_;
    delete preprocess_dialog_;
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
        triangles_label_->setText(QString::number(mesh->triangles().size()));
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
                emit loader_progress(err, scan, percent);
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
    ScanPtr s,
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

    if (!s)
    {
        loader_dialog_->setValue(percent * 100);
        return;
    }

    handle_loader_finish(s);

    loader_dialog_->close();
}

void MainWindow::handle_loader_finish(ScanPtr s)
{
    QString shape_x = "", shape_y = "", shape_z = "";
    double scale_x = 0, scale_y = 0, scale_z = 0;

    if (s)
    {
        scan_ = s;

        shape_x = QString::number(s->shape().x());
        shape_y = QString::number(s->shape().y());
        shape_z = QString::number(s->shape().z());

        scale_x = s->scale().x();
        scale_y = s->scale().y();
        scale_z = s->scale().z();
    }

    ui->labelShapeX->setText(shape_x);
    ui->labelShapeY->setText(shape_y);
    ui->labelShapeZ->setText(shape_z);

    ui->labelVoxelX->setValue(scale_x);
    ui->labelVoxelY->setValue(scale_y);
    ui->labelVoxelZ->setValue(scale_z);

    QCoreApplication::processEvents();
}

void MainWindow::handle_preprocess_progress(
    CGCP::Error err,
    ScanPtr s,
    double percent)
{
    if (preprocess_dialog_->wasCanceled())
    {
        return;
    }

    if (err != CGCP::Error::OK)
    {
        preprocess_dialog_->close();
        QMessageBox msg;
        msg.setText(messageForError(err));
        msg.exec();

        return;
    }

    if (!s)
    {
        preprocess_dialog_->setValue(percent * 100);
        return;
    }

    handle_preprocess_finish(s);

    preprocess_dialog_->close();
};

void MainWindow::handle_cancel_preprocess()
{
    engine_->preprocessor().cancel();
}

void MainWindow::handle_preprocess_finish(ScanPtr scan)
{
    std::shared_ptr<CGCP::ContinuesFunction> f = std::make_shared<CGCP::TIFunction>(
        scan,
        ui->inputIncludeEdges->isChecked(),
        ui->inputIncludeEdges->isChecked());

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

        QMessageBox msg;
        msg.setText(text);
        msg.exec();

        return;
    }

    polygonizer_dialog_->reset();
    polygonizer_dialog_->show();
};

void MainWindow::on_buttonPolygonize_clicked()
{
    try
    {
        engine_->preprocessor().run(
            scan_,
            {{
                 "average",
                 {{"size", fmt::format("{}", ui->inputAverage->value())}},
             },
             {"offset",
              {{"value", fmt::format("{}", -ui->inputOffset->value())}}}},
            [&](
                CGCP::Error err,
                std::shared_ptr<CGCP::TomographyScan> scan,
                double p) -> void
            {
                emit preprocess_progress(err, scan, p);
            });
    }
    catch (CGCP::Exception &e)
    {
        const char *text = e.message().c_str();

        if (e.message().compare("Scan is not set") == 0)
        {
            text = "Нет данных томографии";
        }

        QMessageBox msg;
        msg.setText(text);
        msg.exec();

        return;
    }

    preprocess_dialog_->reset();
    preprocess_dialog_->show();
}

void MainWindow::on_buttonRemoveMesh_clicked()
{
    engine_->drawer().get("main").setMesh(nullptr);
}

void MainWindow::on_buttonChangeVoxel_clicked()
{
    if (!scan_)
    {
        QMessageBox msg;
        msg.setText("Сначала загрузите файл");
        msg.exec();
    }

    scan_->scale().x() = ui->labelVoxelX->value();
    scan_->scale().y() = ui->labelVoxelY->value();
    scan_->scale().z() = ui->labelVoxelZ->value();
}

void MainWindow::on_buttonReset_clicked()
{
    engine_->drawer().get("main").resetTransformation();
}

void MainWindow::on_buttonDrawerConfig_clicked()
{
    std::map<std::string, std::string> config;

    config["shadow"] = QString::number(
                           ui->inputShadowEnabled->isChecked())
                           .toStdString();
    config["border"] = QString::number(
                           ui->inputBorderEnabled->isChecked())
                           .toStdString();

    config["color1"] = ui->inputColor1->color()
                           .name(QColor::HexRgb)
                           .toStdString();
    config["color2"] = ui->inputColor2->color()
                           .name(QColor::HexRgb)
                           .toStdString();
    config["color3"] = ui->inputColor3->color()
                           .name(QColor::HexRgb)
                           .toStdString();
    config["color4"] = ui->inputColor4->color()
                           .name(QColor::HexRgb)
                           .toStdString();
    config["border_color"] = ui->inputColorBorder->color()
                                 .name(QColor::HexRgb)
                                 .toStdString();

    config["light_x"] = QString::number(ui->inputLightX->value()).toStdString();
    config["light_y"] = QString::number(ui->inputLightY->value()).toStdString();
    config["light_z"] = QString::number(ui->inputLightZ->value()).toStdString();

    engine_->drawer().get("main").config(config);
}
