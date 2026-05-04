#ifndef BARCHARTWIDGET_H
#define BARCHARTWIDGET_H

#include <QWidget>
#include <QPropertyAnimation>

struct BarItem {
    QString label;
    double value;
    QColor color;
};

class BarChartWidget : public QWidget {
    Q_OBJECT
    Q_PROPERTY(double animProgress READ animProgress WRITE setAnimProgress)

public:
    explicit BarChartWidget(QWidget *parent = nullptr);
    void setData(const QVector<BarItem> &items, const QString &title = "");

    double animProgress() const { return m_animProgress; }
    void setAnimProgress(double v) { m_animProgress = v; update(); }

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QVector<BarItem> m_items;
    QString m_title;
    double m_animProgress = 0.0;
    QPropertyAnimation *m_anim;
};

#endif // BARCHARTWIDGET_H