#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "src/Engine.h"
#include <QMainWindow>

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

private:
    Ui::MainWindow *ui;
    std::unique_ptr<CGCP::Engine> engine_;
};
#endif // MAINWINDOW_H
