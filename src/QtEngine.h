#pragma once

#include "Engine.h"
#include <QGraphicsView>
#include <memory>

class QtEngine : public CGCP::Engine
{
public:
    QtEngine() = delete;
    QtEngine(QGraphicsView *view);
    virtual ~QtEngine() override = default;
};