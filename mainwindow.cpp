#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "src/Polygon.h"
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QPolygonF>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    CGCP::Polygon p({
        CGCP::Point(0, 0, 0),
        CGCP::Point(100, 0, 0),
        CGCP::Point(0, 100, 0),
    });

    auto view = ui->graphicsView;
    auto scene = new QGraphicsScene(ui->graphicsView);

    view->setScene(scene);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scene->setSceneRect(0, 0, view->geometry().width(), view->geometry().height());

    view->show();

    QPolygonF polygon;

    for (const auto &it : p.points())
    {
        polygon << QPointF(it.x(), it.y());
    }

    scene->addPolygon(polygon);
}

MainWindow::~MainWindow()
{
    delete ui;
}
