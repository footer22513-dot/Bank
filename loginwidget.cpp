// widgets/loginwidget.cpp
#include "loginwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>

LoginWidget::LoginWidget(QWidget *parent) : BaseWidget(parent) {
    auto* mainLayout = new QHBoxLayout(this);

    auto* v1 = new QVBoxLayout;
    mainLayout->addLayout(v1);

    titleLabel = new QLabel("ВХОД");
    titleLabel->setStyleSheet("font-size: 50px");
    titleLabel->setAlignment(Qt::AlignCenter);
    v1->addWidget(titleLabel);

    auto* v2 = new QVBoxLayout;
    mainLayout->addLayout(v2);

    adminButton = new QPushButton("АДМИН");
    adminButton->setStyleSheet("font-size: 32px");
    connect(adminButton, &QPushButton::clicked, this, &LoginWidget::navigateToAdmin);
    v2->addWidget(adminButton);

    userButton = new QPushButton("ПОЛЬЗОВАТЕЛЬ");
    userButton->setStyleSheet("font-size: 32px");
    connect(userButton, &QPushButton::clicked, this, &LoginWidget::navigateToUser);
    v2->addWidget(userButton);

    setLayout(mainLayout);
}
