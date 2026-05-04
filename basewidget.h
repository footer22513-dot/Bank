// widgets/basewidget.h
#ifndef BASEWIDGET_H
#define BASEWIDGET_H

#include <QWidget>

class BaseWidget : public QWidget {
    Q_OBJECT
public:
    explicit BaseWidget(QWidget *parent = nullptr) : QWidget(parent) {}

signals:
    // Сигналы навигации (все виджеты используют одни и те же)
    void navigateToLogin();
    void navigateToAdmin();
    void navigateToUser();
    void navigateToCreditTable();
    void navigateToDepositTable();
    void navigateToCreditForm();
    void navigateToDepositForm();
    void navigateToSuccess();
    void navigateToAnalytics();
    void navigateToMovementCharts();
};

#endif // BASEWIDGET_H
