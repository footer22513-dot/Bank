// widgets/userwidget.h
#ifndef USERWIDGET_H
#define USERWIDGET_H

#include "basewidget.h"

class QPushButton;
class QLabel;

class UserWidget : public BaseWidget {
    Q_OBJECT
public:
    explicit UserWidget(QWidget *parent = nullptr);

private:
    QPushButton* creditButton;      // Переход к форме кредита
    QPushButton* depositButton;     // Переход к форме вклада
    QPushButton* backButton;        // Возврат на экран входа
    QLabel* titleLabel;             // Заголовок "ОПЦИИ"
};

#endif // USERWIDGET_H
