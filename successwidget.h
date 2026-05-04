#ifndef SUCCESSWIDGET_H
#define SUCCESSWIDGET_H

#include <QWidget>

class QLabel;
class QPushButton;

class SuccessWidget : public QWidget {
    Q_OBJECT
public:
    explicit SuccessWidget(QWidget *parent = nullptr);

signals:
    void navigateToUser();

private:
    QLabel* successLabel;
    QPushButton* backBtn;
    void setupUI();
    void setupConnections();
};

#endif // SUCCESSWIDGET_H
