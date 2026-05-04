#include "depositformwidget.h"
#include <QtWidgets>
#include <QtCore>
#include <QtGui>
#include <QtMath>
#include <cmath>

// Предустановленные ставки для вкладов
static const QStringList DEPOSIT_RATES = {"5.00%", "7.50%", "10.00%", "15.00%", "20.00%"};
static const double      DEPOSIT_RATE_VALUES[] = {5.0, 7.5, 10.0, 15.0, 20.0};

DepositFormWidget::DepositFormWidget(QWidget *parent) : QWidget(parent) {
    setupUI();
    setupConnections();
}

void DepositFormWidget::setupUI() {
    auto* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    auto* inner  = new QWidget;
    auto* layout = new QVBoxLayout(inner);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(8);

    QFont lf("Segoe UI", 13, QFont::Bold);
    QFont wf("Segoe UI", 13);

    auto addRow = [&](const QString& label, QWidget* w) {
        auto* lbl = new QLabel(label); lbl->setFont(lf);
        layout->addWidget(lbl);
        w->setFont(wf);
        layout->addWidget(w);
    };

    fioInput      = new QLineEdit;
    phoneInput    = new QLineEdit;
    issueDateEdit = new QDateEdit(QDate::currentDate());
    issueDateEdit->setCalendarPopup(true);
    issueDateEdit->setDisplayFormat("dd.MM.yyyy");

    // Срок — по ТЗ п.3.1.3: 1м, 3м, 6м, 1г, 3г, произвольный
    termCombo = new QComboBox;
    termCombo->addItems({"1 месяц (30 дней)", "3 месяца (90 дней)", "6 месяцев (180 дней)",
                         "1 год (365 дней)", "3 года (1095 дней)", "Произвольный"});
    daysInput = new QLineEdit;
    daysInput->setPlaceholderText("Введите количество дней");
    daysInput->setVisible(false);
    daysInput->setFont(wf);

    rateTypeCombo = new QComboBox;
    rateTypeCombo->addItems({"ФИКСИРОВАННАЯ", "ЗАВИСИТ ОТ СУММЫ", "ЗАВИСИТ ОТ СРОКА"});

    rateCombo = new QComboBox;
    rateCombo->addItems(DEPOSIT_RATES);

    periodCombo = new QComboBox;
    periodCombo->addItems({"ЕЖЕМЕСЯЧНО", "ЕЖЕКВАРТАЛЬНО", "ЕЖЕГОДНО"});

    startSumInput = new QLineEdit;

    addRow("ФИО:", fioInput);
    addRow("НОМЕР ТЕЛЕФОНА:", phoneInput);
    addRow("ДАТА НАЧАЛА:", issueDateEdit);
    addRow("СРОК РАЗМЕЩЕНИЯ:", termCombo);
    layout->addWidget(daysInput);
    addRow("ТИП СТАВКИ:", rateTypeCombo);
    addRow("ПРОЦЕНТНАЯ СТАВКА:", rateCombo);
    addRow("ПЕРИОДИЧНОСТЬ ВЫПЛАТ:", periodCombo);
    addRow("СУММА ВКЛАДА (руб.):", startSumInput);

    auto* endTitle = new QLabel("СУММА К ПОЛУЧЕНИЮ:");
    endTitle->setFont(QFont("Segoe UI", 15, QFont::Bold));
    layout->addWidget(endTitle);

    endSumLabel = new QLabel("0.00 руб.");
    endSumLabel->setStyleSheet("font-size:22px; color:#1976D2; font-weight:bold;"
                               "background:#f0f0f0; padding:8px; border-radius:4px;");
    endSumLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(endSumLabel);
    layout->addStretch();

    createBtn = new QPushButton("ОТКРЫТЬ ВКЛАД");
    createBtn->setFont(QFont("Segoe UI", 18, QFont::Bold));
    createBtn->setMinimumHeight(52);
    createBtn->setStyleSheet("background:#2196F3; color:white; border-radius:6px;");
    layout->addWidget(createBtn);
    // Export analytics button
    exportAnalyticsBtn = new QPushButton("Экспорт аналитики");
    exportAnalyticsBtn->setFont(QFont("Segoe UI", 14));
    exportAnalyticsBtn->setStyleSheet("background:#4CAF50; color:white; border-radius:5px; margin-top:8px;");


    backBtn = new QPushButton("НАЗАД");
    backBtn->setFont(QFont("Segoe UI", 16));
    backBtn->setMinimumHeight(48);
    backBtn->setStyleSheet("background:#f44336; color:white; border-radius:6px;");
    layout->addWidget(backBtn);

    scroll->setWidget(inner);
    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(0,0,0,0);
    outer->addWidget(scroll);
    setLayout(outer);
}

void DepositFormWidget::setupConnections() {
    // FIX: Cast to void(QComboBox::*)(int) to specify the 'int' version
    connect(termCombo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, [this](int idx){
        daysInput->setVisible(idx == termCombo->count()-1);
        calculateEndSum();
    });

    connect(daysInput,     &QLineEdit::textChanged, this, &DepositFormWidget::calculateEndSum);

    // FIX APPLIED HERE: Added the cast for rateCombo as well
    connect(rateCombo,     static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &DepositFormWidget::calculateEndSum);
    connect(rateTypeCombo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &DepositFormWidget::calculateEndSum);

    connect(startSumInput, &QLineEdit::textChanged, this, &DepositFormWidget::calculateEndSum);
    connect(periodCombo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &DepositFormWidget::calculateEndSum);
    connect(createBtn, &QPushButton::clicked, this, &DepositFormWidget::onCreateDepositClicked);
    connect(backBtn,   &QPushButton::clicked, this, &DepositFormWidget::navigateToUser);
    connect(exportAnalyticsBtn, &QPushButton::clicked, this, &DepositFormWidget::onExportAnalyticsClicked);
}

int DepositFormWidget::selectedDays() const {
    static const int p[] = {30, 90, 180, 365, 1095};
    int i = termCombo->currentIndex();
    return (i < 5) ? p[i] : daysInput->text().toInt();
}

double DepositFormWidget::selectedRate() const {
    int i = rateCombo->currentIndex();
    return (i >= 0 && i < 5) ? DEPOSIT_RATE_VALUES[i] : 0.0;
}

void DepositFormWidget::calculateEndSum() {
    bool ok;
    double start = startSumInput->text().toDouble(&ok);
    int    days  = selectedDays();
    if (!(ok && start > 0 && days > 0)) {
        endSumLabel->setText("0.00 руб.");
        return;
    }
    // Determine base rate (annual percent) based on selected rate
    double baseRate = selectedRate();
    // Adjust rate based on rate type selection
    int rateType = rateTypeCombo->currentIndex(); // 0: fixed, 1: depends on sum, 2: depends on term
    if (rateType == 1) { // depends on sum
        // Чем больше сумма, тем выгоднее ставка
        if (start >= 100000) {
            baseRate += 1.5; // > 100k: +1.5%
        } else if (start >= 50000) {
            baseRate += 1.0; // 50-100k: +1.0%
        } else if (start >= 10000) {
            baseRate += 0.5; // 10-50k: +0.5%
        }
        // < 10k: базовая ставка
    } else if (rateType == 2) { // depends on term
        // Чем длиннее срок, тем выгоднее ставка
        if (days >= 730) { // > 2 года
            baseRate += 1.5;
        } else if (days >= 365) { // 1-2 года
            baseRate += 1.0;
        } else if (days >= 180) { // 6-12 месяцев
            baseRate += 0.5;
        }
        // < 6 месяцев: базовая ставка
    }
    // Calculate based on payment period (capitalization frequency)
    int periodIdx = periodCombo->currentIndex(); // 0 monthly, 1 quarterly, 2 yearly
    int periodsPerYear = (periodIdx == 0) ? 12 : (periodIdx == 1) ? 4 : 1;
    double periodRate = baseRate / periodsPerYear / 100.0; // rate per period
    double totalPeriods = days * periodsPerYear / 365.0;   // total number of periods
    // Compound interest: A = P * (1 + r)^n
    double end = start * pow(1.0 + periodRate, totalPeriods);
    endSumLabel->setText(QString::number(end, 'f', 2) + " руб.");
}

void DepositFormWidget::onCreateDepositClicked() {
    if (fioInput->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Укажите ФИО клиента."); return;
    }
    if (phoneInput->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Укажите номер телефона."); return;
    }
    bool ok;
    double start = startSumInput->text().toDouble(&ok);
    if (!ok || start <= 0) {
        QMessageBox::warning(this, "Ошибка", "Сумма вклада должна быть больше нуля."); return;
    }
    // Дополнительные валидации
    if (issueDateEdit->date() > QDate::currentDate()) {
        QMessageBox::warning(this, "Ошибка", "Дата не может быть в будущем.");
        return;
    }
    if (selectedRate() <= 0) {
        QMessageBox::warning(this, "Ошибка", "Ставка проведена.");
        return;
    }
    int days = selectedDays();
    if (days <= 0) {
        QMessageBox::warning(this, "Ошибка", "Укажите срок размещения."); return;
    }
    // Calculate effective rate with sum/term adjustments
    double effectiveRate = selectedRate();
    int rateType = rateTypeCombo->currentIndex();
    if (rateType == 1) { // от суммы
        if (start >= 100000) effectiveRate += 1.5;
        else if (start >= 50000) effectiveRate += 1.0;
        else if (start >= 10000) effectiveRate += 0.5;
    } else if (rateType == 2) { // от срока
        if (days >= 730) effectiveRate += 1.5;
        else if (days >= 365) effectiveRate += 1.0;
        else if (days >= 180) effectiveRate += 0.5;
    }

    // Calculate with capitalization
    int periodIdx = periodCombo->currentIndex();
    int periodsPerYear = (periodIdx == 0) ? 12 : (periodIdx == 1) ? 4 : 1;
    double periodRate = effectiveRate / periodsPerYear / 100.0;
    double totalPeriods = days * periodsPerYear / 365.0;
    double endSum = start * pow(1.0 + periodRate, totalPeriods);

    emit depositDataReady(
        fioInput->text().trimmed(), phoneInput->text().trimmed(),
        issueDateEdit->date(), days,
        rateTypeCombo->currentIndex(), effectiveRate,
        periodCombo->currentIndex(),
        start, endSum
        );
    emit navigateToSuccess();
    // Reset form fields after successful submission
    fioInput->clear();
    phoneInput->clear();
    issueDateEdit->setDate(QDate::currentDate());
    termCombo->setCurrentIndex(0);
    daysInput->clear();
    daysInput->setVisible(false);
    rateTypeCombo->setCurrentIndex(0);
    rateCombo->setCurrentIndex(0);
    periodCombo->setCurrentIndex(0);
    startSumInput->clear();
    endSumLabel->setText("0.00 руб.");
}

void DepositFormWidget::onExportAnalyticsClicked() {
    // Simple text report of current form values
    QString filename = QDir::homePath() + "/deposit_report.txt";
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Ошибка", "Не удалось создать файл отчёта.");
        return;
    }
    QTextStream out(&file);
    out << "Отчёт по вкладу\n";
    out << "ФИО: " << fioInput->text() << "\n";
    out << "Телефон: " << phoneInput->text() << "\n";
    out << "Дата начала: " << issueDateEdit->date().toString("dd.MM.yyyy") << "\n";
    out << "Срок (дней): " << selectedDays() << "\n";
    out << "Тип ставки: " << rateTypeCombo->currentText() << "\n";
    out << "Ставка: " << rateCombo->currentText() << "\n";
    out << "Периодичность: " << periodCombo->currentText() << "\n";
    out << "Сумма вклада: " << startSumInput->text() << "\n";
    out << "Сумма к получению: " << endSumLabel->text() << "\n";
    file.close();
    QMessageBox::information(this, "Экспорт", "Отчёт сохранён в " + filename);
}