// widgets/userwidget.cpp
#include "userwidget.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>

UserWidget::UserWidget(QWidget *parent)
    : BaseWidget(parent)
{
    // Основной вертикальный макет
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(15);

    // Заголовок
    titleLabel = new QLabel("ОПЦИИ ПОЛЬЗОВАТЕЛЯ");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 50px; font-weight: bold;");
    layout->addWidget(titleLabel);

    // Пустое пространство для центрирования кнопок
    layout->addStretch(1);

    // Кнопка "КРЕДИТ"
    creditButton = new QPushButton(" ОФОРМИТЬ КРЕДИТ");
    creditButton->setStyleSheet(
        "QPushButton {"
        "   font-size: 32px;"
        "   padding: 20px;"
        "   background-color: #4CAF50;"
        "   color: white;"
        "   border-radius: 10px;"
        "}"
        "QPushButton:hover { background-color: #45a049; }"
        "QPushButton:pressed { background-color: #3d8b40; }"
        );
    connect(creditButton, &QPushButton::clicked, this, &UserWidget::navigateToCreditForm);
    layout->addWidget(creditButton);

    // Кнопка "ВКЛАД"
    depositButton = new QPushButton("ОТКРЫТЬ ВКЛАД");
    depositButton->setStyleSheet(
        "QPushButton {"
        "   font-size: 32px;"
        "   padding: 20px;"
        "   background-color: #2196F3;"
        "   color: white;"
        "   border-radius: 10px;"
        "}"
        "QPushButton:hover { background-color: #1e88e5; }"
        "QPushButton:pressed { background-color: #1976d2; }"
        );
    connect(depositButton, &QPushButton::clicked, this, &UserWidget::navigateToDepositForm);
    layout->addWidget(depositButton);

    layout->addStretch(1);

    // Кнопка "НАЗАД"
    backButton = new QPushButton("⬅ НАЗАД");
    backButton->setStyleSheet(
        "QPushButton {"
        "   font-size: 24px;"
        "   padding: 15px;"
        "   background-color: #f44336;"
        "   color: white;"
        "   border-radius: 8px;"
        "}"
        "QPushButton:hover { background-color: #e53935; }"
        );
    connect(backButton, &QPushButton::clicked, this, &UserWidget::navigateToLogin);
    layout->addWidget(backButton);

    setLayout(layout);
}
