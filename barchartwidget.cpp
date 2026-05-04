#include "barchartwidget.h"
#include <QPainter>

BarChartWidget::BarChartWidget(QWidget *parent) : QWidget(parent) {
    m_anim = new QPropertyAnimation(this, "animProgress", this);
    m_anim->setDuration(1000);
    m_anim->setEasingCurve(QEasingCurve::OutCubic);
}

void BarChartWidget::setData(const QVector<BarItem> &items, const QString &title) {
    m_items = items;
    m_title = title;
    m_anim->stop();
    m_anim->setStartValue(0.0);
    m_anim->setEndValue(1.0);
    m_anim->start();
}

void BarChartWidget::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // 1. Настройки отступов
    int margin = 40;
    QRect chartRect = rect().adjusted(margin, margin, -margin, -margin);

    // Рисуем заголовок
    if (!m_title.isEmpty()) {
        p.setFont(QFont("Arial", 12, QFont::Bold));
        p.drawText(rect().adjusted(0, 5, 0, -chartRect.height()), Qt::AlignCenter, m_title);
    }

    if (m_items.isEmpty()) return;

    // 2. Расчет масштаба
    double maxVal = 0;
    for (const auto &it : m_items) if (it.value > maxVal) maxVal = it.value;
    if (maxVal <= 0) maxVal = 1;

    // 3. Рисуем оси и сетку (3 линии)
    p.setPen(QColor(200, 200, 200));
    for (int i = 0; i <= 4; ++i) {
        int y = chartRect.bottom() - (chartRect.height() * i / 4);
        p.drawLine(chartRect.left(), y, chartRect.right(), y);
        p.drawText(chartRect.left() - 35, y + 5, QString::number(maxVal * i / 4, 'f', 0));
    }

    // 4. Рисуем столбцы
    double barWidth = (double)chartRect.width() / m_items.size();
    p.setFont(QFont("Arial", 9));

    for (int i = 0; i < m_items.size(); ++i) {
        // Расчет геометрии столбца
        double h = (m_items[i].value / maxVal) * chartRect.height() * m_animProgress;
        int x = chartRect.left() + i * barWidth + barWidth * 0.1;
        int w = barWidth * 0.8;
        int y = chartRect.bottom() - h;

        QRect bar(x, y, w, h);

        // Цвет и заливка
        p.setPen(Qt::NoPen);
        p.setBrush(m_items[i].color);
        p.drawRect(bar);

        // Текст под столбцом
        p.setPen(Qt::black);
        p.drawText(QRect(x, chartRect.bottom() + 5, w, 20), Qt::AlignCenter, m_items[i].label);

        // Значение над столбцом (появляется в конце анимации)
        if (m_animProgress > 0.8) {
            p.drawText(QRect(x, y - 20, w, 20), Qt::AlignCenter, QString::number(m_items[i].value));
        }
    }
}