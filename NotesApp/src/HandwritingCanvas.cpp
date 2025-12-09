#include "HandwritingCanvas.h"

#include <QMouseEvent>
#include <QPainter>

HandwritingCanvas::HandwritingCanvas(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_StaticContents);
    setMinimumHeight(260);
}

void HandwritingCanvas::mousePressEvent(QMouseEvent *e)
{
    if (!isEnabled()) {
        QWidget::mousePressEvent(e);
        return;
    }

    path.moveTo(e->pos());
    update();
}

void HandwritingCanvas::mouseMoveEvent(QMouseEvent *e)
{
    if (!isEnabled() || !(e->buttons() & Qt::LeftButton)) {
        QWidget::mouseMoveEvent(e);
        return;
    }

    path.lineTo(e->pos());
    update();
}

void HandwritingCanvas::mouseReleaseEvent(QMouseEvent *e)
{
    if (!isEnabled()) {
        QWidget::mouseReleaseEvent(e);
        return;
    }

    path.lineTo(e->pos());
    update();
    emit strokeFinished();
}

void HandwritingCanvas::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    // background + border
    p.fillRect(rect(), QColor(250, 250, 250));
    p.setPen(QPen(QColor(210, 210, 210), 1));
    p.drawRect(rect().adjusted(0, 0, -1, -1));

    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(Qt::black, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p.drawPath(path);
}

void HandwritingCanvas::clearCanvas()
{
    path = QPainterPath();
    update();
}

QImage HandwritingCanvas::toImage() const
{
    QImage img(size(), QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::white);

    QPainter p(&img);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(Qt::black, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p.drawPath(path);
    return img;
}
