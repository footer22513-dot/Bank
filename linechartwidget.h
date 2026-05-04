#ifndef LINECHARTWIDGET_H
#define LINECHARTWIDGET_H

#include <QWidget>
#include <QPropertyAnimation>
#include <QDate>
#include <QVector>

struct TimePoint {
    QDate date;
    double creditSum;
    double depositSum;
};

class LineChartWidget : public QWidget {
    Q_OBJECT
    Q_PROPERTY(double animProgress READ animProgress WRITE setAnimProgress)

public:
    explicit LineChartWidget(QWidget *parent = nullptr);
    void setData(const QVector<TimePoint> &points);

    double animProgress() const { return m_animProgress; }
    void setAnimProgress(double v) { m_animProgress = v; update(); }

protected:
    void paintEvent(QPaintEvent *event) override;

public:
    void clear();

private:
    QVector<TimePoint> m_points;
    double m_animProgress = 0.0;
    QPropertyAnimation *m_anim;
};

#endif