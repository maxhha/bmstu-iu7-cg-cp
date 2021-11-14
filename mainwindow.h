#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "src/Engine.h"
#include <QLabel>
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
    using ScanPtr = std::shared_ptr<CGCP::TomographyScan>;
    using MeshPtr = std::shared_ptr<CGCP::Mesh>;

signals:
    void polygonizer_progress(MeshPtr mesh, double percent);

    void loader_progress(CGCP::Error err, ScanPtr f, double percent);

    void preprocess_progress(CGCP::Error err, ScanPtr f, double percent);

    void saver_progress(CGCP::Error err, bool done, double percent);

private slots:
    void on_buttonTranslate_clicked();

    void on_buttonScale_clicked();

    void on_buttonRotate_clicked();

    void handle_polygonizer_progress(MeshPtr mesh, double percent);

    void handle_loader_progress(CGCP::Error err, ScanPtr s, double percent);

    void handle_preprocess_progress(CGCP::Error err, ScanPtr s, double percent);

    void handle_saver_progress(CGCP::Error err, bool done, double percent);

    void handle_cancel_polygonizer();

    void handle_cancel_loader();

    void handle_cancel_saver();

    void handle_cancel_preprocess();

    void on_buttonOpen_clicked();

    void on_buttonPolygonize_clicked();

    void on_buttonReset_clicked();

    void on_buttonRemoveMesh_clicked();

    void on_buttonChangeVoxel_clicked();

    void on_buttonDrawerConfig_clicked();

    void on_buttonSaveMesh_clicked();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void handle_loader_finish(ScanPtr s);
    void handle_preprocess_finish(ScanPtr s);

private:
    Ui::MainWindow *ui;
    std::unique_ptr<CGCP::Engine> engine_;
    ScanPtr scan_;
    MeshPtr mesh_;
    QProgressDialog *polygonizer_dialog_;
    QProgressDialog *preprocess_dialog_;
    QProgressDialog *loader_dialog_;
    QProgressDialog *saver_dialog_;
    QLabel *triangles_label_;
};
#endif // MAINWINDOW_H
