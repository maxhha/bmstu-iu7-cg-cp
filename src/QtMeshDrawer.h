#pragma once

#include "MeshDrawer.h"
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMatrix4x4>
#include <QWidget>

class QtMeshDrawer : public QWidget, public CGCP::MeshDrawer
{
    Q_OBJECT

private:
    QGraphicsView *view_;
    QGraphicsScene *scene_;
    QVector3D scale_{1, 1, 1};
    QVector3D translate_;
    QMatrix4x4 rotate_;
    QMatrix4x4 projection_;

    void drawMesh();
    QPointF transform(const CGCP::Vec3Df &p);

protected:
    void resizeEvent(QResizeEvent *event) override;

public:
    QtMeshDrawer() = delete;
    explicit QtMeshDrawer(QGraphicsView *frame);

    virtual void setMesh(const std::shared_ptr<CGCP::Mesh> mesh) override;

    virtual void rotate(const CGCP::Vec3Df &axis, double phi) override;
    virtual void translate(const CGCP::Vec3Df &offset) override;
    virtual void scale(const CGCP::Vec3Df &scale) override;

    virtual ~QtMeshDrawer() override;
};