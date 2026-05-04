#include "linechartwidget.h"
#include <QWidget>
#include <QtGui/QPainter>
#include <QtGui/QPainterPath>
#include <QtGui/QPaintEvent>
#include <QtCore/QVector>
#include <QtCore/QDate>
#include <QtCore/QPropertyAnimation>
#include <QtGui/QPen>
#include <QtGui/QColor>
#include <QtCore/QString>
#include <QPainter>
#include <QVector>
#include <QPaintEvent>
#include <QPainterPath>
#include <QWidget>
#include <QSizePolicy>
#include <QEasingCurve>
#include <QVector>
#include <QSize>
#include <QRectF>
#include <QFont>
#include <QPen>
#include <QColor>
#include <QString>
#include <QPaintEvent>
#include <QPropertyAnimation>

LineChartWidget::LineChartWidget(QWidget *parent) : QWidget(parent) {
    m_anim = new QPropertyAnimation(this, "animProgress", this);
    m_anim->setDuration(1200);
    m_anim->setEasingCurve(QEasingCurve::InOutQuad);
}

void LineChartWidget::setData(const QVector<TimePoint> &points) {
    m_points = points;
    m_anim->stop();
    m_anim->setStartValue(0.0);
    m_anim->setEndValue(1.0);
    m_anim->start();
    update();
}

void LineChartWidget::clear() {
    m_points.clear();
    m_animProgress = 1.0;
    update();
}

void LineChartWidget::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // 1. Настройки области рисования
    int marginLeft = 60, marginRight = 20, marginTop = 30, marginBottom = 60;
    QRect r = rect().adjusted(marginLeft, marginTop, -marginRight, -marginBottom);

    if (m_points.isEmpty()) {
        p.drawText(rect(), Qt::AlignCenter, "Нет данных");
        return;
    }

    // 2. Поиск максимума для масштабирования
    double maxVal = 1.0;
    for (const auto &pt : m_points) {
        maxVal = qMax(maxVal, qMax(pt.creditSum, pt.depositSum));
    }
    maxVal *= 1.1; // запас сверху

    // 3. Отрисовка осей и сетки
    p.setPen(QColor(220, 220, 220));
    for (int i = 0; i <= 4; ++i) {
        int y = r.bottom() - (r.height() * i / 4);
        p.drawLine(r.left(), y, r.right(), y);
        p.setPen(Qt::gray);
        p.drawText(r.left() - 55, y + 5, 50, 15, Qt::AlignRight, QString::number(maxVal * i / 4, 'f', 0));
    }

    // 4. Отрисовка линий (Кредиты - Оранжевый, Вклады - Зеленый)
    auto drawSeries = [&](bool isCredit) {
        QColor color = isCredit ? QColor("#FFB347") : QColor("#06D6A0");
        QPainterPath path;
        double stepX = (double)r.width() / qMax(1, m_points.size() - 1);

        for (int i = 0; i < m_points.size(); ++i) {
            double val = isCredit ? m_points[i].creditSum : m_points[i].depositSum;
            double x = r.left() + i * stepX;
            double y = r.bottom() - (val / maxVal * r.height());

            if (i == 0) path.moveTo(x, y);
            else path.lineTo(x, y);
        }

        // Эффект анимации через обрезку (Clip)
        p.save();
        p.setClipRect(0, 0, r.left() + r.width() * m_animProgress, height());

        // Рисуем заливку
        QPainterPath fillPath = path;
        fillPath.lineTo(r.left() + (m_points.size()-1) * stepX, r.bottom());
        fillPath.lineTo(r.left(), r.bottom());
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(color.red(), color.green(), color.blue(), 40));
        p.drawPath(fillPath);

        // Рисуем саму линию
        p.setPen(QPen(color, 2));
        p.setBrush(Qt::NoBrush);
        p.drawPath(path);

        // Рисуем точки
        for (int i = 0; i < m_points.size(); ++i) {
            double val = isCredit ? m_points[i].creditSum : m_points[i].depositSum;
            QPointF pt(r.left() + i * stepX, r.bottom() - (val / maxVal * r.height()));
            p.setBrush(Qt::white);
            p.setPen(color);
            p.drawEllipse(pt, 3, 3);
        }
        p.restore();
    };

    drawSeries(true);  // Кредиты
    drawSeries(false); // Вклады

    // 5. Подписи дат по оси X
    p.setPen(Qt::darkGray);
    int step = qMax(1, m_points.size() / 5);
    for (int i = 0; i < m_points.size(); i += step) {
        int x = r.left() + i * ((double)r.width() / qMax(1, m_points.size() - 1));
        p.drawText(x - 30, r.bottom() + 10, 60, 20, Qt::AlignCenter, m_points[i].date.toString("dd.MM"));
    }

    // 6. Простая легенда внизу
    int legY = height() - 30;
    p.setPen(Qt::NoPen);
    p.setBrush(QColor("#FFB347")); p.drawRect(r.left(), legY, 12, 12);
    p.setPen(Qt::black); p.drawText(r.left() + 18, legY + 11, "Кредиты");

    p.setPen(Qt::NoPen);
    p.setBrush(QColor("#06D6A0")); p.drawRect(r.left() + 100, legY, 12, 12);
    p.setPen(Qt::black); p.drawText(r.left() + 118, legY + 11, "Вклады");
}