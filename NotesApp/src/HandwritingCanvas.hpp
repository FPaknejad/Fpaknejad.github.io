#pragma once

#include <QWidget>
#include <QPainterPath>
#include <QImage>

class HandwritingCanvas : public QWidget
{
    Q_OBJECT

public:
    explicit HandwritingCanvas(QWidget *parent = nullptr);

    void clearCanvas();
    QImage toImage() const; // for future real recognition

signals:
    void strokeFinished();

protected:
    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void paintEvent(QPaintEvent* e) override;

private:
    QPainterPath path;
};
