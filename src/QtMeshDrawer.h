#pragma once

#include "MeshDrawer.h"
#include <QGraphicsScene>
#include <QGraphicsView>

class QtMeshDrawer : public CGCP::MeshDrawer
{
private:
    QGraphicsView *view_;
    QGraphicsScene *scene_;

public:
    explicit QtMeshDrawer(QGraphicsView *view);

    virtual void setMesh(std::shared_ptr<CGCP::Mesh> mesh) override;

    virtual ~QtMeshDrawer() override;
};