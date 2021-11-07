#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "src/Engine.h"
#include <QMainWindow>
#include <QProgressDialog>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    using FunctionPtr = std::shared_ptr<CGCP::ContinuesFunction>;
    using MeshPtr = std::shared_ptr<CGCP::Mesh>;

signals:
    void polygonizer_progress(MeshPtr mesh, double percent);

    void loader_progress(CGCP::Error err, FunctionPtr f, double percent);

private slots:
    void on_buttonTranslate_clicked();

    void on_buttonScale_clicked();

    void on_buttonRotate_clicked();

    void handle_polygonizer_progress(MeshPtr mesh, double percent);

    void handle_loader_progress(CGCP::Error err, FunctionPtr f, double percent);

    void handle_cancel_polygonizer();

    void handle_cancel_loader();

    void on_buttonOpen_clicked();

    void on_buttonPolygonize_clicked();

    void on_buttonReset_clicked();

    void on_buttonRemoveMesh_clicked();

    void on_buttonChangeVoxel_clicked();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void handle_loader_finish(FunctionPtr f);
    FunctionPtr compose_function();

private:
    Ui::MainWindow *ui;
    std::unique_ptr<CGCP::Engine> engine_;
    QProgressDialog *polygonizer_dialog_;
    QProgressDialog *loader_dialog_;
};
#endif // MAINWINDOW_H
