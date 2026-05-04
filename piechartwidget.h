// piechartwidget.h
#ifndef PIECHARTWIDGET_H
#define PIECHARTWIDGET_H

#include <QWidget>
#include <QPropertyAnimation>

class PieChartWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(double animProgress READ animProgress WRITE setAnimProgress)

public:
    explicit PieChartWidget(QWidget *parent = nullptr);

    void setValues(int credits, int deposits);
    void setEmpty();
    void setShowPercentage(bool show);
    void updateData(int credits, int deposits);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent *) override;
    void resizeEvent(QResizeEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;
    void leaveEvent(QEvent *) override;

private:
    struct Segment {
        QString label;
        int realValue = 0;      // целевое значение
        double drawValue = 0;   // текущее для анимации
        QColor base;
        QColor light;
        QColor dark;
    };

    static constexpr qreal HOLE_RATIO = 0.60;
    static constexpr qreal EXPAND_HOVER = 1.04;

    double animProgress() const { return m_animProgress; }
    void setAnimProgress(double v);

    QRectF chartRect() const;
    int hitTest(const QPoint &pos) const;
    double angleAt(const QPointF &center, const QPoint &pos) const;

    void drawDropShadow(QPainter &p, const QRectF &r);
    void drawSlices(QPainter &p, const QRectF &r);
    void drawCenterInfo(QPainter &p, const QRectF &r);
    void drawExternalLabels(QPainter &p, const QRectF &r);

    Segment m_seg[2];
    bool m_isEmpty = true;
    bool m_showPercentage = true;
    int m_hover = -1;
    double m_animProgress = 0.0;

    QPropertyAnimation *m_anim;
};

#endif