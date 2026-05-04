#include "successwidget.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFont>

SuccessWidget::SuccessWidget(QWidget *parent) : QWidget(parent) {
    setupUI();
    setupConnections();
}

void SuccessWidget::setupUI() {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(40, 40, 40, 40);

    successLabel = new QLabel("УСПЕШНО!");
    successLabel->setAlignment(Qt::AlignCenter);
    successLabel->setStyleSheet("font-size: 64px; font-weight: bold; color: #2e7d32;");
    layout->addWidget(successLabel);

    QLabel* subLabel = new QLabel("Операция успешно сохранена в системе.");
    subLabel->setAlignment(Qt::AlignCenter);
    subLabel->setStyleSheet("font-size: 20px; color: #555;");
    layout->addWidget(subLabel);

    layout->addStretch();

    backBtn = new QPushButton("НАЗАД В МЕНЮ");
    backBtn->setFont(QFont("Segoe UI", 24, QFont::Bold));
    backBtn->setMinimumHeight(60);
    backBtn->setStyleSheet("background-color: #2196F3; color: white; border-radius: 8px;");
    layout->addWidget(backBtn);
}

void SuccessWidget::setupConnections() {
    connect(backBtn, &QPushButton::clicked, this, &SuccessWidget::navigateToUser);
}
