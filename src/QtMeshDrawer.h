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
    QVector3D light_direction_{1, 2, -3};
    QMatrix4x4 light_projection_;
    QVector3D light_translate_;
    QVector3D light_scale_;
    std::mt19937 color_rg_;
    std::uniform_real_distribution<double> urd_normal_;
    QImage color_buffer_;
    std::unique_ptr<double[]> z_buffer_;
    std::unique_ptr<double[]> light_buffer_;
    int z_buffer_w_ = 0;
    int z_buffer_h_ = 0;
    int light_buffer_w_;
    int light_buffer_h_;
    bool shadow_enabled_ = false;
    QColor dark_color_[2] = {
        QColor("#200036"),
        QColor("#100018")};
    QColor light_color_[2] = {
        QColor("#888088"),
        QColor("#200036")};

    QColor border_color_ = QColor("#ff3333");
    bool border_enabled_ = false;

    void draw();
    void drawMesh();
    void drawMeshWireframe();
    void drawAxis();

    void drawTriangle(
        double *z_buf,
        int w, int h,
        const CGCP::Triangle3Df &it,
        std::function<QVector3D(const CGCP::Vec3Df &)> transform,
        std::function<void(const CGCP::Vec3Df &p, int x, int y, bool b)> set_color,
        double limit_z);

    void updateBuffers();
    void updateLightMatrix();

    QVector3D transform(const CGCP::Vec3Df &p);
    QVector3D transformMesh(const CGCP::Vec3Df &p);
    QVector3D transformLight(const CGCP::Vec3Df &p);
    bool isInLight(const CGCP::Vec3Df &p);
    QColor color(const CGCP::Triangle3Df &p, bool shadow);

protected:
    using MeshDrawer::Config;
    void resizeEvent(QResizeEvent *event) override;

public:
    QtMeshDrawer() = delete;
    explicit QtMeshDrawer(QGraphicsView *frame);

    virtual void setMesh(const std::shared_ptr<CGCP::Mesh> mesh) override;

    virtual void config(const Config &config) override;
    virtual void resetTransformation() override;
    virtual void rotate(const CGCP::Vec3Df &axis, double phi) override;
    virtual void translate(const CGCP::Vec3Df &offset) override;
    virtual void scale(const CGCP::Vec3Df &scale) override;

    virtual ~QtMeshDrawer() override;
};
