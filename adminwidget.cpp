#include "adminwidget.h"
#include "bankrecord.h"
#include "creditcalc.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QDate>
#include <QDir>
#include <QMap>
#include <QSet>
#include <algorithm>
AdminWidget::AdminWidget(QWidget *parent) : BaseWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(40, 30, 40, 30);
    layout->setSpacing(14);

    titleLabel = new QLabel("ПАНЕЛЬ АДМИНИСТРАТОРА");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size:44px; font-weight:bold;");
    layout->addWidget(titleLabel);
    layout->addStretch();

    auto btn = [&](const QString& text, const QString& color) {
        auto* b = new QPushButton(text);
        b->setMinimumHeight(54);
        b->setStyleSheet(QString("font-size:26px; background:%1; color:white; border-radius:8px;").arg(color));
        return b;
    };

    analytics       = btn("АНАЛИТИКА", "#6A1B9A");
    creditTableBtn  = btn("📋  ТАБЛИЦА ЗАЙМОВ",   "#388E3C");
    depositTableBtn = btn("💰  ТАБЛИЦА ВКЛАДОВ",  "#1976D2");
    loadDbBtn       = btn("📂  ЗАГРУЗИТЬ БАЗУ",   "#F57C00");
    saveDbBtn       = btn("💾  ВЫГРУЗИТЬ БАЗУ",   "#6A1B9A");
    exportBtn       = btn("📄  ЭКСПОРТ АНАЛИТИКИ", "#5E35B1");
    backBtn         = btn("← НАЗАД",              "#555555");

    connect(analytics,       &QPushButton::clicked, this, &AdminWidget::navigateToAnalytics);
    connect(creditTableBtn,  &QPushButton::clicked, this, &AdminWidget::navigateToCreditTable);
    connect(depositTableBtn, &QPushButton::clicked, this, &AdminWidget::navigateToDepositTable);
    connect(loadDbBtn,       &QPushButton::clicked, this, &AdminWidget::loadDatabaseRequested);
    connect(saveDbBtn,       &QPushButton::clicked, this, &AdminWidget::saveDatabaseRequested);
    connect(exportBtn,       &QPushButton::clicked, this, &AdminWidget::onExportAnalyticsClicked);
    connect(backBtn,         &QPushButton::clicked, this, &AdminWidget::navigateToLogin);

    layout->addWidget(analytics);
    layout->addWidget(creditTableBtn);
    layout->addWidget(depositTableBtn);
    layout->addSpacing(8);
    layout->addWidget(loadDbBtn);
    layout->addWidget(saveDbBtn);
    layout->addWidget(exportBtn);
    layout->addStretch();
    layout->addWidget(backBtn);
    setLayout(layout);
}

void AdminWidget::setRecords(const QList<BankRecord>* records) {
    m_records = records;
}

void AdminWidget::onExportAnalyticsClicked() {
    if (!m_records) {
        QMessageBox::warning(this, "Ошибка", "Данные не загружены.");
        return;
    }

    QString path = QFileDialog::getSaveFileName(
        this, "Сохранить аналитический отчёт",
        QDir::homePath() + "/analytics_report.txt",
        "Текстовые файлы (*.txt)");

    if (path.isEmpty()) return;

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Ошибка", "Не удалось создать файл отчёта.");
        return;
    }

    QTextStream out(&file);
    QDate today = QDate::currentDate();

    /* ---------- сбор статистики ---------- */
    int totalCount = 0, creditCount = 0, depositCount = 0;
    double creditSum = 0.0, depositSum = 0.0;
    double creditRateSum = 0.0, depositRateSum = 0.0;
    int creditRateCnt = 0, depositRateCnt = 0;
    int creditTermSum = 0, depositTermSum = 0;

    int overdueCount = 0;
    double overdueDebt = 0.0, overduePenalty = 0.0;

    QMap<int, int>    rateTypeCnt;
    QMap<int, double> rateTypeSum;
    QMap<QString, double> clientSum;
    QMap<QString, QString> clientPhone;

    QMap<QString, QPair<double,int>> monCred;
    QMap<QString, QPair<double,int>> monDep;

    for (const auto &r : *m_records) {
        ++totalCount;
        bool isCred = (r.type == OperationType::Credit);

        if (isCred) {
            ++creditCount;
            creditSum += r.startSum;
            creditRateSum += r.ratePercent; ++creditRateCnt;
            creditTermSum += r.termDays;
            monCred[r.issueDate.toString("MM.yyyy")].first  += r.startSum;
            monCred[r.issueDate.toString("MM.yyyy")].second += 1;

            if (r.issueDate.isValid()) {
                QDate due = r.issueDate.addDays(r.termDays);
                if (due < today && r.endSum > 0.0) {
                    ++overdueCount;
                    overdueDebt += r.endSum;
                    int daysLate = due.daysTo(today);
                    if (daysLate > 0)
                        overduePenalty += CreditCalculator::calculatePenalty(
                            r.endSum, r.penaltyPercent, r.penaltyAmount, daysLate, static_cast<int>(r.paymentPeriod));
                }
            }
        } else {
            ++depositCount;
            depositSum += r.startSum;
            depositRateSum += r.ratePercent; ++depositRateCnt;
            depositTermSum += r.termDays;
            monDep[r.issueDate.toString("MM.yyyy")].first  += r.startSum;
            monDep[r.issueDate.toString("MM.yyyy")].second += 1;
        }

        int rt = static_cast<int>(r.rateType);
        ++rateTypeCnt[rt];
        rateTypeSum[rt] += r.startSum;

        QString name = r.fullName.trimmed();
        clientSum[name] += r.startSum;
        clientPhone[name] = r.phone;
    }

    /* ---------- вывод отчёта ---------- */
    auto f2 = [](double v){ return QString::number(v, 'f', 2); };

    out << "═══════════════════════════════════════════════════════\n";
    out << "         АНАЛИТИЧЕСКИЙ ОТЧЁТ ПО СИСТЕМЕ\n";
    out << "═══════════════════════════════════════════════════════\n";
    out << "Дата формирования: " << today.toString("dd.MM.yyyy") << "\n";
    out << "Всего операций:    " << totalCount << "\n\n";

    out << "─── ОБЩАЯ СТАТИСТИКА ─────────────────────────────────\n";
    out << QString("Кредиты:   %1 шт. на сумму %2 руб.\n")
               .arg(creditCount).arg(f2(creditSum));
    out << QString("Вклады:    %1 шт. на сумму %2 руб.\n")
               .arg(depositCount).arg(f2(depositSum));
    out << QString("Итого оборот: %1 руб.\n\n")
               .arg(f2(creditSum + depositSum));

    out << "─── СРЕДНИЕ ПОКАЗАТЕЛИ ───────────────────────────────\n";
    out << QString("Средняя ставка (кредиты): %1%\n")
               .arg(creditRateCnt ? f2(creditRateSum / creditRateCnt) : "0.00");
    out << QString("Средняя ставка (вклады):  %1%\n")
               .arg(depositRateCnt ? f2(depositRateSum / depositRateCnt) : "0.00");
    out << QString("Средний срок (кредиты):   %1 дн.\n")
               .arg(creditCount ? QString::number(creditTermSum / creditCount) : "0");
    out << QString("Средний срок (вклады):    %1 дн.\n\n")
               .arg(depositCount ? QString::number(depositTermSum / depositCount) : "0");

    out << "─── ПРОСРОЧЕННЫЕ КРЕДИТЫ ─────────────────────────────\n";
    out << QString("Количество:          %1\n").arg(overdueCount);
    out << QString("Сумма задолженности: %1 руб.\n").arg(f2(overdueDebt));
    out << QString("Начисленные штрафы:  %1 руб.\n").arg(f2(overduePenalty));
    if (overdueCount > 0) {
        out << "\nСписок просроченных:\n";
        out << "ID   | ФИО                 | Телефон      | Выдан      | Срок | Проср. | Остаток      | Штраф\n";
        out << "────────────────────────────────────────────────────────────────────────────────────────────\n";
        for (const auto &r : *m_records) {
            if (r.type != OperationType::Credit || !r.issueDate.isValid()) continue;
            QDate due = r.issueDate.addDays(r.termDays);
            if (due < today && r.endSum > 0.0) {
                int dl = due.daysTo(today);
                double pen = (dl > 0) ? CreditCalculator::calculatePenalty(r.endSum, r.penaltyPercent, r.penaltyAmount, dl, static_cast<int>(r.paymentPeriod)) : 0.0;
                out << QString("%1 | %2 | %3 | %4 | %5 | %6 | %7 | %8\n")
                           .arg(r.id, 4)
                           .arg(r.fullName.left(18), -20)
                           .arg(r.phone, -12)
                           .arg(r.issueDate.toString("dd.MM.yyyy"), -10)
                           .arg(r.termDays, 4)
                           .arg(dl, 6)
                           .arg(f2(r.endSum), 12)
                           .arg(f2(pen), 10);
            }
        }
    }
    out << "\n";

    out << "─── РАСПРЕДЕЛЕНИЕ ПО ТИПАМ СТАВОК ─────────────────────\n";
    QStringList rn = {"Фиксированная", "От суммы", "От срока"};
    for (int i = 0; i < 3; ++i) {
        out << QString("%1: %2 руб. (%3 операций)\n")
                   .arg(rn[i], -15)
                   .arg(f2(rateTypeSum.value(i, 0.0)), 12)
                   .arg(rateTypeCnt.value(i, 0), 4);
    }
    out << "\n";

    out << "─── ТОП-5 КЛИЕНТОВ ПО СУММЕ ОПЕРАЦИЙ ──────────────────\n";
    QList<QPair<QString,double>> top;
    for (auto it = clientSum.begin(); it != clientSum.end(); ++it)
        top.append({it.key(), it.value()});
    std::sort(top.begin(), top.end(),
              [](const auto &a, const auto &b){ return a.second > b.second; });

    for (int i = 0; i < qMin(5, top.size()); ++i) {
        out << QString("%1. %2 | %3 | %4 руб.\n")
                   .arg(i + 1)
                   .arg(top[i].first)
                   .arg(clientPhone.value(top[i].first, "-"))
                   .arg(f2(top[i].second));
    }
    out << "\n";

    out << "─── ДИНАМИКА ПО МЕСЯЦАМ ──────────────────────────────\n";
    QSet<QString> ms;
    for (auto it = monCred.begin(); it != monCred.end(); ++it) ms.insert(it.key());
    for (auto it = monDep.begin(); it != monDep.end(); ++it) ms.insert(it.key());
    QStringList months = ms.values();
    std::sort(months.begin(), months.end(),
              [](const QString &a, const QString &b){
                  return QDate::fromString(a+".01","MM.yyyy.dd") <
                         QDate::fromString(b+".01","MM.yyyy.dd");
              });

    out << QString("%1 | %2 | %3\n")
               .arg("Месяц", -10)
               .arg("Кредиты (сумма / шт)", -25)
               .arg("Вклады (сумма / шт)", -25);
    out << "────────────────────────────────────────────────────────────\n";
    for (const QString &m : months) {
        auto c = monCred.value(m, {0.0, 0});
        auto d = monDep.value(m, {0.0, 0});
        out << QString("%1 | %2 / %3 | %4 / %5\n")
                   .arg(m, -10)
                   .arg(f2(c.first), 14).arg(c.second, 4)
                   .arg(f2(d.first), 14).arg(d.second, 4);
    }

    out << "\n═══════════════════════════════════════════════════════\n";
    out << "                    Конец отчёта\n";
    out << "═══════════════════════════════════════════════════════\n";

    file.close();
    QMessageBox::information(this, "Готово",
                             QString("Аналитический отчёт сохранён:\n%1").arg(path));
}
