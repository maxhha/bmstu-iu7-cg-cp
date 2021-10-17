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

signals:
    void polygonizer_progress(double percent);

private slots:
    void on_buttonTranslate_clicked();

    void on_buttonScale_clicked();

    void on_buttonRotate_clicked();

    void handle_polygonizer_progress(double percent);

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    Ui::MainWindow *ui;
    std::unique_ptr<CGCP::Engine> engine_;
    QProgressDialog *dialog_;
};
#endif // MAINWINDOW_H
