// widgets/loginwidget.h
#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H

#include "basewidget.h"

class QPushButton;
class QLabel;

class LoginWidget : public BaseWidget {
    Q_OBJECT
public:
    explicit LoginWidget(QWidget *parent = nullptr);

private:
    QPushButton* adminButton;
    QPushButton* userButton;
    QLabel* titleLabel;
};
#endif // LOGINWIDGET_H
