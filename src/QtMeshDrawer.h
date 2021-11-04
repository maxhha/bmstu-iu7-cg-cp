#pragma once

#include "MeshDrawer.h"
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QImage>
#include <QMatrix4x4>
#include <QWidget>
#include <random>

class QtMeshDrawer : public QWidget, public CGCP::MeshDrawer
{
    Q_OBJECT

private:
    QGraphicsView *view_;
    QGraphicsScene *scene_;
    QVector3D scale_{1, 1, 1};
    QVector3D translate_;
    QVector3D base_;
    QMatrix4x4 rotate_;
    QMatrix4x4 projection_;
    QVector3D light_direction{1, 2, -3};
    std::mt19937 color_rg_;
    std::uniform_real_distribution<double> urd_normal_;
    QImage color_buffer_;
    std::unique_ptr<double[]> z_buffer_;

    void draw();
    void drawMesh();
    void drawMeshWireframe();

    void drawTriangle(QImage &color_buffer, double *z_buffer, const CGCP::Triangle3Df &it, const QColor &color);

    void updateBuffers();

    QVector3D transform(const CGCP::Vec3Df &p);
    QVector3D transformMesh(const CGCP::Vec3Df &p);
    QColor color(const CGCP::Triangle3Df &p);

protected:
    void resizeEvent(QResizeEvent *event) override;

public:
    QtMeshDrawer() = delete;
    explicit QtMeshDrawer(QGraphicsView *frame);

    virtual void setMesh(const std::shared_ptr<CGCP::Mesh> mesh) override;

    virtual void resetTransformation() override;
    virtual void rotate(const CGCP::Vec3Df &axis, double phi) override;
    virtual void translate(const CGCP::Vec3Df &offset) override;
    virtual void scale(const CGCP::Vec3Df &scale) override;

    virtual ~QtMeshDrawer() override;
};