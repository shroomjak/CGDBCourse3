#ifndef ZOOMABLEGRAPHICSVIEW_H
#define ZOOMABLEGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QMouseEvent>
#include <QWheelEvent>

class ZoomableGraphicsView : public QGraphicsView
{
    Q_OBJECT

public:
    explicit ZoomableGraphicsView(QWidget *parent = nullptr)
        : QGraphicsView(parent), scaleFactor(1.0), isDragging(false)
    {
        setDragMode(QGraphicsView::NoDrag); // Перетаскивание будет обрабатываться вручную
        setTransformationAnchor(QGraphicsView::AnchorUnderMouse); // Масштабирование относительно курсора
        setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    }

protected:
    // Масштабирование колесом мыши
    void wheelEvent(QWheelEvent *event) override
    {
        if (event->angleDelta().y() > 0) {
            zoomIn();
        } else {
            zoomOut();
        }
    }

    // Обработка нажатия мыши (начало перетаскивания)
    void mousePressEvent(QMouseEvent *event) override
    {
        if (event->button() == Qt::LeftButton) {
            isDragging = true;
            dragStartPosition = event->pos();
            setCursor(Qt::ClosedHandCursor); // Меняем курсор на "руку"
        }
        QGraphicsView::mousePressEvent(event);
    }

    // Обработка движения мыши (перетаскивание карты)
    void mouseMoveEvent(QMouseEvent *event) override
    {
        if (isDragging) {
            QPointF delta = mapToScene(event->pos()) - mapToScene(dragStartPosition);
            translate(-delta.x(), -delta.y()); // Перемещение содержимого
            dragStartPosition = event->pos();
        }
        QGraphicsView::mouseMoveEvent(event);
    }

    // Обработка отпускания мыши (завершение перетаскивания)
    void mouseReleaseEvent(QMouseEvent *event) override
    {
        if (event->button() == Qt::LeftButton) {
            isDragging = false;
            setCursor(Qt::ArrowCursor); // Возвращаем стандартный курсор
        }
        QGraphicsView::mouseReleaseEvent(event);
    }

private:
    qreal scaleFactor;
    bool isDragging;
    QPoint dragStartPosition;

    void zoomIn()
    {
        if (scaleFactor <= 5.0) { // Ограничиваем максимальное приближение
            scale(1.1, 1.1);
            scaleFactor *= 1.1;
        }
    }

    void zoomOut()
    {
        if (scaleFactor >= 1.0) { // Ограничиваем минимальное отдаление
            scale(1 / 1.1, 1 / 1.1);
            scaleFactor /= 1.1;
        }
    }
};

#endif // ZOOMABLEGRAPHICSVIEW_H
