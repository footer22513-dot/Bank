// piechartwidget.cpp
#include "piechartwidget.h"
#include <QPainter>
#include <QFontDatabase>
#include <QMouseEvent>
#include <QtMath>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

PieChartWidget::PieChartWidget(QWidget *parent)
    : QWidget(parent), m_anim(new QPropertyAnimation(this, "animProgress", this))
{
    setMouseTracking(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // --- палитра ---
    m_seg[0].label = "КРЕДИТЫ";
    m_seg[0].base  = QColor("#FFB347");  // тёплый золотой
    m_seg[0].light = QColor("#FFD700");
    m_seg[0].dark  = QColor("#E8963A");

    m_seg[1].label = "ВКЛАДЫ";
    m_seg[1].base  = QColor("#06D6A0");  // мятный
    m_seg[1].light = QColor("#4FFFD3");
    m_seg[1].dark  = QColor("#05B086");

    // --- анимация ---
    m_anim->setDuration(900);
    m_anim->setEasingCurve(QEasingCurve::OutCubic);

    setEmpty();
}

void PieChartWidget::setValues(int credits, int deposits)
{
    // existing implementation
    if (m_seg[0].realValue == credits && m_seg[1].realValue == deposits && !m_isEmpty)
        return;

    m_isEmpty = false;
    m_seg[0].realValue = credits;
    m_seg[1].realValue = deposits;

    m_anim->stop();
    m_animProgress = 0.0;
    m_anim->setStartValue(0.0);
    m_anim->setEndValue(1.0);
    m_anim->start();
    update();
}

void PieChartWidget::setEmpty()
{
    m_isEmpty = true;
    m_seg[0].realValue = 0;
    m_seg[1].realValue = 0;
    m_seg[0].drawValue = 0;
    m_seg[1].drawValue = 0;
    m_animProgress = 1.0;
    update();
}

void PieChartWidget::setShowPercentage(bool show)
{
    m_showPercentage = show;
    update();
}

QSize PieChartWidget::sizeHint() const { return QSize(520, 400); }
QSize PieChartWidget::minimumSizeHint() const { return QSize(280, 220); }

QRectF PieChartWidget::chartRect() const
{
    qreal side = qMin(width(), height()) * 0.72;
    QRectF r(0, 0, side, side);
    r.moveCenter(rect().center());
    return r;
}

double PieChartWidget::angleAt(const QPointF &center, const QPoint &pos) const
{
    double dx = pos.x() - center.x();
    double dy = center.y() - pos.y(); // Y инвертирован в Qt
    double deg = qRadiansToDegrees(std::atan2(dy, dx));
    if (deg < 0) deg += 360.0;
    // Qt рисует pie от 90° против часовой → приводим к той же системе
    double qt0 = 90.0 - deg;
    if (qt0 < 0) qt0 += 360.0;
    return qt0;
}

int PieChartWidget::hitTest(const QPoint &pos) const
{
    QRectF r = chartRect();
    QPointF c = r.center();
    double dx = pos.x() - c.x();
    double dy = pos.y() - c.y();
    double dist = std::sqrt(dx*dx + dy*dy);
    double outer = r.width() / 2.0;
    double inner = outer * HOLE_RATIO;

    if (dist < inner || dist > outer * EXPAND_HOVER)
        return -1;

    if (m_isEmpty) return -1;

    double total = m_seg[0].drawValue + m_seg[1].drawValue;
    if (total <= 0) return -1;

    double a = angleAt(c, pos); // 0..360, от 90° Qt

    double a1 = (m_seg[0].drawValue / total) * 360.0;
    if (a <= a1) return 0;
    return 1;
}

void PieChartWidget::mouseMoveEvent(QMouseEvent *e)
{
    int h = hitTest(e->pos());
    if (h != m_hover) {
        m_hover = h;
        update();
    }
}

void PieChartWidget::leaveEvent(QEvent *)
{
    if (m_hover != -1) {
        m_hover = -1;
        update();
    }
}

void PieChartWidget::setAnimProgress(double v)
{
    m_animProgress = v;
    // интерполируем drawValue
    static double start0 = 0, start1 = 0;
    if (v <= 0.0) {
        start0 = m_seg[0].drawValue;
        start1 = m_seg[1].drawValue;
    }
    if (!m_isEmpty) {
        m_seg[0].drawValue = start0 + (m_seg[0].realValue - start0) * v;
        m_seg[1].drawValue = start1 + (m_seg[1].realValue - start1) * v;
    }
    update();
}

// Update chart data, used by MainWindow
void PieChartWidget::updateData(int credits, int deposits)
{
    if (credits == 0 && deposits == 0) {
        setEmpty();
    } else {
        setValues(credits, deposits);
    }
}

void PieChartWidget::drawDropShadow(QPainter &p, const QRectF &r)
{
    // мягкая размытая тень вручную (без QGraphicsEffect)
    for (int i = 5; i >= 0; --i) {
        qreal off = i * 1.5;
        qreal alpha = 18 - i * 2;
        QRectF sr = r.translated(off, off + 2);
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(0, 0, 0, qMax(qreal(0), alpha)));
        p.drawEllipse(sr);
    }
}

void PieChartWidget::drawSlices(QPainter &p, const QRectF &r)
{
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);

    double total = m_seg[0].drawValue + m_seg[1].drawValue;
    bool empty = m_isEmpty || total <= 0;

    // фоновое кольцо для «пустого» состояния
    if (empty) {
        QConicalGradient cg(r.center(), 90);
        cg.setColorAt(0.0, QColor(80, 80, 100, 80));
        cg.setColorAt(0.5, QColor(60, 60, 80, 80));
        cg.setColorAt(1.0, QColor(80, 80, 100, 80));
        p.setBrush(cg);
        p.drawPie(r, 0, 360 * 16);
    } else {
        double start = 90.0; // Qt начинает сверху
        for (int i = 0; i < 2; ++i) {
            double frac = (total > 0) ? (m_seg[i].drawValue / total) : 0;
            double span = frac * 360.0;
            if (span < 0.5) continue;

            bool hover = (m_hover == i);
            QRectF rr = r;
            if (hover) {
                rr.adjust(-4, -4, 4, 4);
            }

            // градиент даёт объём
            QConicalGradient grad(rr.center(), -start);
            QColor c1 = m_seg[i].light;
            QColor c2 = m_seg[i].dark;
            if (hover) {
                c1 = c1.lighter(115);
                c2 = c2.lighter(115);
            }
            grad.setColorAt(0.0, c1);
            grad.setColorAt(1.0, c2);
            p.setBrush(grad);
            p.drawPie(rr, start * 16, span * 16);

            // тонкая светлая граница между сегментами
            QPen pen(QColor(255,255,255,120));
            pen.setWidthF(hover ? 2.5 : 1.5);
            p.setPen(pen);
            p.drawArc(rr, start * 16, span * 16);
            p.setPen(Qt::NoPen);

            start += span;
        }
    }

    // центральное отверстие (donut)
    double hole = r.width() * HOLE_RATIO;
    QRectF hr(0, 0, hole, hole);
    hr.moveCenter(r.center());
    p.setBrush(palette().color(QPalette::Window));
    p.drawEllipse(hr);

    // тонкий ободок вокруг дырки для чёткости
    QPen outline(palette().color(QPalette::Mid), 1.0);
    p.setPen(outline);
    p.setBrush(Qt::NoBrush);
    p.drawEllipse(hr);
}

void PieChartWidget::drawCenterInfo(QPainter &p, const QRectF &r)
{
    double hole = r.width() * HOLE_RATIO;
    QRectF hr(0, 0, hole * 0.85, hole * 0.85);
    hr.moveCenter(r.center());

    if (m_isEmpty) {
        QFont f = font();
        f.setPointSize(qMax(9, qRound(r.width() / 18)));
        f.setBold(true);
        p.setFont(f);
        p.setPen(QColor(150, 150, 170));
        p.drawText(hr, Qt::AlignCenter, "Нет данных");
        return;
    }

    double total = m_seg[0].drawValue + m_seg[1].drawValue;
    int itotal = qRound(total);

    QFont f = font();
    f.setPointSize(qMax(10, qRound(r.width() / 12)));
    f.setBold(true);
    p.setFont(f);
    p.setPen(palette().color(QPalette::WindowText));

    if (m_showPercentage && total > 0) {
        // показываем сумму крупно и проценты мелко
        QString big = QString::number(itotal);
        QFontMetrics fm(f);
        QRectF tr = fm.boundingRect(big);
        tr.moveCenter(hr.center() - QPointF(0, tr.height()*0.2));
        p.drawText(tr, Qt::AlignCenter, big);

        f.setPointSize(qMax(8, qRound(r.width() / 22)));
        f.setBold(false);
        p.setFont(f);
        QString sub = "всего";
        p.drawText(hr.translated(0, tr.height()*0.6), Qt::AlignCenter, sub);
    } else {
        QString txt = QString::number(itotal);
        p.drawText(hr, Qt::AlignCenter, txt);
    }
}

void PieChartWidget::drawExternalLabels(QPainter &p, const QRectF &r)
{
    if (m_isEmpty) return;

    double total = m_seg[0].drawValue + m_seg[1].drawValue;
    if (total <= 0) return;

    p.setRenderHint(QPainter::Antialiasing);

    double startQt = 90.0;
    for (int i = 0; i < 2; ++i) {
        double frac = m_seg[i].drawValue / total;
        if (frac < 0.02) { startQt += frac * 360.0; continue; }

        double span = frac * 360.0;
        double mid = startQt + span / 2.0;
        double rad = qDegreesToRadians(-mid + 90.0); // обратно в матем. угол

        double outR = r.width() / 2.0 + 10;
        QPointF c = r.center();
        QPointF elbow(c.x() + outR * std::cos(rad),
                      c.y() - outR * std::sin(rad));

        // линия-выноска
        double labelR = outR + 28;
        QPointF tip(c.x() + labelR * std::cos(rad),
                    c.y() - labelR * std::sin(rad));

        bool left = (tip.x() < c.x());
        QPointF textPos = tip + QPointF(left ? -8 : 8, 0);

        // рисуем линию
        QPen linePen(QColor(180, 180, 200), 1.2);
        p.setPen(linePen);
        p.drawLine(elbow, tip);
        p.drawLine(tip, tip + QPointF(left ? -6 : 6, 0));

        // текст
        QFont f = font();
        f.setPointSize(qMax(9, qRound(r.width() / 20)));
        f.setBold(true);
        p.setFont(f);
        QString line1 = m_seg[i].label;
        QString line2 = QString::number(qRound(m_seg[i].drawValue));
        if (m_showPercentage)
            line2 += QString(" (%1%)").arg(QString::number(frac * 100.0, 'f', 1));

        QRectF tr = p.boundingRect(QRectF(), Qt::AlignLeft, line1 + "\n" + line2);
        QRectF box(textPos.x() + (left ? -tr.width() : 0),
                   textPos.y() - tr.height() / 2.0,
                   tr.width() + 10, tr.height() + 8);

        // фон подписи
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(255, 255, 255, qMin(240, 180 + (m_hover == i ? 40 : 0))));
        p.drawRoundedRect(box, 6, 6);

        // обводка при наведении
        if (m_hover == i) {
            p.setPen(QPen(m_seg[i].base, 1.5));
            p.setBrush(Qt::NoBrush);
            p.drawRoundedRect(box, 6, 6);
        }

        p.setPen(QColor(60, 60, 80));
        p.drawText(box, Qt::AlignCenter, line1 + "\n" + line2);
        p.setPen(Qt::NoPen);

        startQt += span;
    }
}

void PieChartWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.fillRect(rect(), Qt::transparent);

    QRectF r = chartRect();
    drawDropShadow(p, r);
    drawSlices(p, r);
    drawCenterInfo(p, r);
    drawExternalLabels(p, r);
}

void PieChartWidget::resizeEvent(QResizeEvent *)
{
    update();
}