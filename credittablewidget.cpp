#include "credittablewidget.h"
#include "creditcalc.h"
#include <QtWidgets>
#include <QStyledItemDelegate>
#include <QLineEdit>
#include <QIntValidator>
#include <QDoubleValidator>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QFileDialog>
#include <QDir>
#include <QtMath>
#include <QRegularExpression>

// Делегат с валидаторами для числовых колонок
class NumericDelegate : public QStyledItemDelegate {
public:
    explicit NumericDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override {
        QLineEdit *editor = new QLineEdit(parent);
        switch (index.column()) {
        case 4: // Срок (дн.) – целое число
            editor->setValidator(new QIntValidator(1, 36500, editor));
            break;
        case 6: // Ставка (%) – двойное, 2 знака после запятой
            editor->setValidator(new QDoubleValidator(0, 1000, 2, editor));
            break;
        case 8: // Нач. сумма
        case 9: // Кон. сумма
        case 10: // Штраф (руб.)
            editor->setValidator(new QDoubleValidator(0, 999999999999.99, 2, editor));
            break;
        case 11: // Штраф (%)
            editor->setValidator(new QDoubleValidator(0, 1000, 2, editor));
            break;
        default:
            break;
        }
        return editor;
    }
};

// Класс для числовой сортировки
class NumericTableItem : public QTableWidgetItem {
public:
    NumericTableItem(const QString& text) : QTableWidgetItem(text) {}

    bool operator<(const QTableWidgetItem& other) const override {
        bool ok1 = false, ok2 = false;
        double v1 = text().toDouble(&ok1);
        double v2 = other.text().toDouble(&ok2);
        if (ok1 && ok2) return v1 < v2;
        return QTableWidgetItem::operator<(other);
    }
};

// Класс для сортировки дат (формат dd.MM.yyyy)
class DateTableItem : public QTableWidgetItem {
public:
    DateTableItem(const QString& text) : QTableWidgetItem(text) {}

    bool operator<(const QTableWidgetItem& other) const override {
        QDate d1 = QDate::fromString(text(), "dd.MM.yyyy");
        QDate d2 = QDate::fromString(other.text(), "dd.MM.yyyy");
        if (d1.isValid() && d2.isValid()) return d1 < d2;
        return QTableWidgetItem::operator<(other);
    }
};

CreditTableWidget::CreditTableWidget(QWidget *parent) : BaseWidget(parent) {
    auto* main = new QHBoxLayout(this);

    // Таблица
    table = new QTableWidget;
    table->setColumnCount(13);
    table->setHorizontalHeaderLabels({
        "ID", "ФИО", "Телефон", "Дата выдачи",
        "Срок (дн.)", "Тип ставки", "Ставка (%)",
        "Периодичность", "Нач. сумма", "Кон. сумма",
        "Штраф (руб.)", "Штраф (%)", "Начислено штрафа"
    });
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    table->horizontalHeader()->setStretchLastSection(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);

    auto* numericDelegate = new NumericDelegate(this);
    table->setItemDelegateForColumn(4, numericDelegate);
    table->setItemDelegateForColumn(6, numericDelegate);
    table->setItemDelegateForColumn(8, numericDelegate);
    table->setItemDelegateForColumn(9, numericDelegate);
    table->setItemDelegateForColumn(10, numericDelegate);
    table->setItemDelegateForColumn(11, numericDelegate);

    table->setSortingEnabled(true);
    main->addWidget(table, 3);

    // Панель управления
    auto* ctrl = new QVBoxLayout;
    main->addLayout(ctrl, 1);

    ctrl->addWidget(new QLabel("Поиск (ФИО / тел. / ID):"));
    searchInput = new QLineEdit;
    searchInput->setPlaceholderText("Введите...");
    ctrl->addWidget(searchInput);

    ctrl->addWidget(new QLabel("Сумма от:"));
    sumMin = new QSpinBox; sumMin->setRange(0, 999999999);
    ctrl->addWidget(sumMin);
    ctrl->addWidget(new QLabel("Сумма до:"));
    sumMax = new QSpinBox; sumMax->setRange(0, 999999999); sumMax->setValue(999999999);
    ctrl->addWidget(sumMax);

    ctrl->addWidget(new QLabel("Дата выдачи от:"));
    dateFrom = new QDateEdit(QDate(2000,1,1)); dateFrom->setCalendarPopup(true);
    dateFrom->setDisplayFormat("dd.MM.yyyy");
    ctrl->addWidget(dateFrom);
    ctrl->addWidget(new QLabel("Дата выдачи до:"));
    dateTo = new QDateEdit(QDate::currentDate().addYears(10)); dateTo->setCalendarPopup(true);
    dateTo->setDisplayFormat("dd.MM.yyyy");
    ctrl->addWidget(dateTo);

    ctrl->addWidget(new QLabel("Фильтр:"));
    overdueOnly = new QCheckBox("Только просроченные");
    overdueOnly->setStyleSheet("font-size:13px; padding:4px;");
    connect(overdueOnly, &QCheckBox::toggled, this, &CreditTableWidget::onSearchClicked);
    ctrl->addWidget(overdueOnly);

    searchBtn = new QPushButton("🔍 ПОИСК");
    searchBtn->setStyleSheet("font-size:14px;padding:8px;background:#388E3C;color:white;border-radius:5px;");
    connect(searchBtn, &QPushButton::clicked, this, &CreditTableWidget::onSearchClicked);
    ctrl->addWidget(searchBtn);

    resetBtn = new QPushButton("СБРОСИТЬ");
    resetBtn->setStyleSheet("font-size:14px;padding:6px;background:#78909C;color:white;border-radius:5px;");
    connect(resetBtn, &QPushButton::clicked, this, &CreditTableWidget::onResetClicked);
    ctrl->addWidget(resetBtn);

    ctrl->addStretch();

    delBtn = new QPushButton("🗑 УДАЛИТЬ");
    delBtn->setStyleSheet("font-size:14px;padding:8px;background:#f44336;color:white;border-radius:5px;");
    connect(delBtn, &QPushButton::clicked, this, &CreditTableWidget::onDeleteClicked);
    ctrl->addWidget(delBtn);

    exportBtn = new QPushButton("📄 Отчёт");
    exportBtn->setStyleSheet("font-size:14px;padding:8px;background:#1565C0;color:white;border-radius:5px;");
    connect(exportBtn, &QPushButton::clicked, this, &CreditTableWidget::onExportReport);
    ctrl->addWidget(exportBtn);

    repayBtn = new QPushButton("💰 Погасить");
    repayBtn->setStyleSheet("font-size:14px;padding:8px;background:#388E3C;color:white;border-radius:5px;");
    connect(repayBtn, &QPushButton::clicked, this, &CreditTableWidget::onEarlyRepayClicked);
    ctrl->addWidget(repayBtn);

    backBtn = new QPushButton("← НАЗАД");
    backBtn->setStyleSheet("font-size:14px;padding:8px;background:#555;color:white;border-radius:5px;");
    connect(backBtn, &QPushButton::clicked, this, &CreditTableWidget::navigateToAdmin);
    ctrl->addWidget(backBtn);

    connect(table, &QTableWidget::itemChanged, this, &CreditTableWidget::onItemChanged);

    setLayout(main);
}

void CreditTableWidget::setRecords(const QList<CreditRecord>& data) {
    records = data;
    refreshTable(records);
}

QList<CreditRecord> CreditTableWidget::getRecords() const {
    return records;
}

static void setRowBackgroundStatus(QTableWidget* table, int row, bool overdue, bool repaid) {
    Q_UNUSED(overdue)
    if (repaid) {
        QColor green(200, 255, 200);
        for (int col = 0; col < table->columnCount(); ++col) {
            if (QTableWidgetItem* it = table->item(row, col))
                it->setBackground(green);
        }
    }
}

void CreditTableWidget::refreshTable(const QList<CreditRecord>& data) {
    m_isRefreshing = true;
    table->setSortingEnabled(false);
    table->setRowCount(0);

    QDate today = QDate::currentDate();

    for (const auto& r : data) {
        int row = table->rowCount();
        table->insertRow(row);

        QDate issueDate = QDate::fromString(r.issueDate, "dd.MM.yyyy");
        bool overdue = false;
        if (issueDate.isValid()) {
            QDate dueDate = issueDate.addDays(r.termDays);
            overdue = (dueDate < today) && (r.endSum > 0);
        }
        bool repaid = qFuzzyIsNull(r.endSum);

        QTableWidgetItem* idItem = new QTableWidgetItem(QString::number(r.id));
        idItem->setFlags(idItem->flags() & ~Qt::ItemIsEditable);
        table->setItem(row, 0, idItem);

        table->setItem(row, 1, new QTableWidgetItem(r.fullName));
        table->setItem(row, 2, new QTableWidgetItem(r.phone));
        table->setItem(row, 3, new DateTableItem(r.issueDate));

        NumericTableItem* termItem = new NumericTableItem(QString::number(r.termDays));
        table->setItem(row, 4, termItem);

        QTableWidgetItem* rateTypeItem = new QTableWidgetItem(rateTypeName(r.rateType));
        rateTypeItem->setFlags(rateTypeItem->flags() & ~Qt::ItemIsEditable);
        table->setItem(row, 5, rateTypeItem);

        table->setItem(row, 6, new NumericTableItem(QString::number(r.ratePercent, 'f', 2)));

        QTableWidgetItem* periodItem = new QTableWidgetItem(periodName(r.paymentPeriod));
        periodItem->setFlags(periodItem->flags() & ~Qt::ItemIsEditable);
        table->setItem(row, 7, periodItem);

        table->setItem(row, 8, new NumericTableItem(QString::number(r.startSum, 'f', 2)));
        table->setItem(row, 9, new NumericTableItem(QString::number(r.endSum, 'f', 2)));

        // Колонка 10: исходная фиксированная сумма штрафа (за период)
        table->setItem(row, 10, new NumericTableItem(QString::number(r.penaltyAmount, 'f', 2)));
        // Колонка 11: процент штрафа (годовой)
        table->setItem(row, 11, new NumericTableItem(QString::number(r.penaltyPercent, 'f', 2)));

        // Колонка 12: вычисленный накопленный штраф (только для просмотра)
        double penaltyAmt = 0.0; // total accrued penalty
        if (overdue) {
            int daysLate = issueDate.addDays(r.termDays).daysTo(today);
            if (daysLate > 0) {
                penaltyAmt = CreditCalculator::calculatePenalty(
                    r.endSum, r.penaltyPercent, r.penaltyAmount, daysLate, r.paymentPeriod);
            }
        }
        if (penaltyAmt > 100000000.0) penaltyAmt = 100000000.0;
        // Вычитаем уже уплаченный штраф
        double displayedPenalty = qMax(0.0, penaltyAmt - r.paidPenalty);
        QTableWidgetItem* accruedItem = new NumericTableItem(QString::number(displayedPenalty, 'f', 2));
        accruedItem->setFlags(accruedItem->flags() & ~Qt::ItemIsEditable);
        if (overdue)
            accruedItem->setBackground(QColor(255, 235, 238)); // розовый для просрочки
        table->setItem(row, 12, accruedItem);

        setRowBackgroundStatus(table, row, overdue, repaid);
    }

    table->setSortingEnabled(true);
    m_isRefreshing = false;
}

void CreditTableWidget::onSearchClicked() {
    QString f = searchInput->text().trimmed().toLower();
    double  minS = sumMin->value(), maxS = sumMax->value();
    QDate   df = dateFrom->date(), dt = dateTo->date();
    bool    onlyOverdue = overdueOnly->isChecked();
    QDate   today = QDate::currentDate();

    QList<CreditRecord> out;
    for (const auto& r : records) {
        QDate d = QDate::fromString(r.issueDate, "dd.MM.yyyy");

        bool nameOk = f.isEmpty() || r.fullName.toLower().contains(f)
                      || r.phone.contains(f)
                      || QString::number(r.id).contains(f);
        bool sumOk  = r.startSum >= minS && r.startSum <= maxS;
        bool dateOk = d.isValid() && (d >= df && d <= dt);

        bool overdue = false;
        if (d.isValid()) {
            QDate dueDate = d.addDays(r.termDays);
            overdue = (dueDate < today) && (r.endSum > 0);
        }
        bool overdueOk = !onlyOverdue || overdue;

        if (nameOk && sumOk && dateOk && overdueOk) out.append(r);
    }
    refreshTable(out);
}

void CreditTableWidget::onResetClicked() {
    searchInput->clear();
    sumMin->setValue(0); sumMax->setValue(999999999);
    dateFrom->setDate(QDate(2000,1,1)); dateTo->setDate(QDate::currentDate().addYears(10));
    overdueOnly->setChecked(false);
    refreshTable(records);
}

void CreditTableWidget::onDeleteClicked() {
    QModelIndexList selected = table->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        QMessageBox::warning(this, "Внимание", "Выберите хотя бы одну строку.");
        return;
    }

    if (QMessageBox::Yes != QMessageBox::question(this, "Удаление",
                                                  QString("Удалить выбранные записи (%1 шт.)?").arg(selected.count()),
                                                  QMessageBox::Yes | QMessageBox::No)) {
        return;
    }

    for (const QModelIndex &idx : selected) {
        int row = idx.row();
        if (QTableWidgetItem* idItem = table->item(row, 0)) {
            int id = idItem->text().toInt();
            for (auto it = records.begin(); it != records.end(); ) {
                if (it->id == id) {
                    it = records.erase(it);
                    emit recordDeleted(id);
                } else {
                    ++it;
                }
            }
        }
    }
    onSearchClicked();
}

static bool validateFullName(const QString& name, QString& error) {
    if (name.trimmed().isEmpty()) {
        error = "ФИО не может быть пустым";
        return false;
    }
    QRegularExpression re("^[\\p{L}\\s\\-]+$");
    if (!re.match(name).hasMatch()) {
        error = "ФИО может содержать только буквы, пробел и дефис";
        return false;
    }
    return true;
}

static bool validatePhone(const QString& phone, QString& error) {
    if (phone.trimmed().isEmpty()) {
        error = "Телефон не может быть пустым";
        return false;
    }
    QRegularExpression re("^[\\+\\d\\s\\-\\(\\)]+$");
    if (!re.match(phone).hasMatch()) {
        error = "Телефон может содержать только цифры, +, пробел, скобки и дефис";
        return false;
    }
    int digitCount = 0;
    for (const QChar& c : phone) {
        if (c.isDigit()) ++digitCount;
    }
    if (digitCount < 5) {
        error = "Телефон слишком короткий (минимум 5 цифр)";
        return false;
    }
    return true;
}

static bool validateDate(const QString& dateStr, QString& error) {
    QDate date = QDate::fromString(dateStr, "dd.MM.yyyy");
    if (!date.isValid()) {
        error = "Неверный формат даты. Используйте ДД.ММ.ГГГГ";
        return false;
    }
    if (date.year() < 1900 || date.year() > 2100) {
        error = "Год должен быть в диапазоне 1900-2100";
        return false;
    }
    return true;
}

static bool validateTermDays(const QString& text, int& outDays, QString& error) {
    bool ok = false;
    int days = text.toInt(&ok);
    if (!ok || days <= 0) {
        error = "Срок должен быть положительным целым числом";
        return false;
    }
    if (days > 36500) {
        error = "Срок не может превышать 100 лет (36500 дней)";
        return false;
    }
    outDays = days;
    return true;
}

static bool validateRate(double rate, QString& error) {
    if (rate < 0) {
        error = "Ставка не может быть отрицательной";
        return false;
    }
    if (rate > 1000) {
        error = "Ставка не может превышать 1000%";
        return false;
    }
    return true;
}

static bool validateSum(double sum, QString& error) {
    if (sum < 0) {
        error = "Сумма не может быть отрицательной";
        return false;
    }
    if (sum > 999999999999.99) {
        error = "Сумма слишком большая";
        return false;
    }
    return true;
}

void CreditTableWidget::onItemChanged(QTableWidgetItem* item) {
    if (m_isRefreshing) return;

    int row = item->row();
    int col = item->column();
    if (row < 0 || row >= table->rowCount()) return;

    int id = table->item(row, 0)->text().toInt();
    auto it = std::find_if(records.begin(), records.end(),
                           [id](const CreditRecord& r) { return r.id == id; });
    if (it == records.end()) return;

    QString newVal = item->text();
    QString errorMsg;
    bool recalcEndSum = false;
    bool recalcPenalty = false;

    switch(col) {
    case 1:
        if (!validateFullName(newVal, errorMsg)) {
            QMessageBox::warning(this, "Ошибка валидации", errorMsg);
            m_isRefreshing = true; item->setText(it->fullName); m_isRefreshing = false;
            return;
        }
        it->fullName = newVal;
        break;
    case 2:
        if (!validatePhone(newVal, errorMsg)) {
            QMessageBox::warning(this, "Ошибка валидации", errorMsg);
            m_isRefreshing = true; item->setText(it->phone); m_isRefreshing = false;
            return;
        }
        it->phone = newVal;
        break;
    case 3:
        if (!validateDate(newVal, errorMsg)) {
            QMessageBox::warning(this, "Ошибка валидации", errorMsg);
            m_isRefreshing = true; item->setText(it->issueDate); m_isRefreshing = false;
            return;
        }
        it->issueDate = newVal;
        recalcPenalty = true;
        break;
    case 4: {
        int days = 0;
        if (!validateTermDays(newVal, days, errorMsg)) {
            QMessageBox::warning(this, "Ошибка валидации", errorMsg);
            m_isRefreshing = true; item->setText(QString::number(it->termDays)); m_isRefreshing = false;
            return;
        }
        it->termDays = days;
        recalcEndSum = true;
        recalcPenalty = true;
        break;
    }
    case 5:
        return;
    case 6: {
        double rate = newVal.toDouble();
        if (!validateRate(rate, errorMsg)) {
            QMessageBox::warning(this, "Ошибка валидации", errorMsg);
            m_isRefreshing = true; item->setText(QString::number(it->ratePercent, 'f', 2)); m_isRefreshing = false;
            return;
        }
        it->ratePercent = rate;
        recalcEndSum = true;
        recalcPenalty = true;
        break;
    }
    case 7:
        return;
    case 8: {
        double sum = newVal.toDouble();
        if (!validateSum(sum, errorMsg)) {
            QMessageBox::warning(this, "Ошибка валидации", errorMsg);
            m_isRefreshing = true; item->setText(QString::number(it->startSum, 'f', 2)); m_isRefreshing = false;
            return;
        }
        it->startSum = sum;
        recalcEndSum = true;
        recalcPenalty = true;
        break;
    }
    case 9: {
        double sum = newVal.toDouble();
        if (!validateSum(sum, errorMsg)) {
            QMessageBox::warning(this, "Ошибка валидации", errorMsg);
            m_isRefreshing = true; item->setText(QString::number(it->endSum, 'f', 2)); m_isRefreshing = false;
            return;
        }
        it->endSum = sum;
        recalcPenalty = true;
        break;
    }
    case 10: {
        double sum = newVal.toDouble();
        if (!validateSum(sum, errorMsg)) {
            QMessageBox::warning(this, "Ошибка валидации", errorMsg);
            m_isRefreshing = true; item->setText(QString::number(it->penaltyAmount, 'f', 2)); m_isRefreshing = false;
            return;
        }
        // Сначала проверяем, не обнуляем ли оба штрафа
        if (qFuzzyIsNull(sum) && qFuzzyIsNull(it->penaltyPercent)) {
            QMessageBox::warning(this, "Ошибка валидации", "Введите либо фиксированный штраф, либо процентный.");
            m_isRefreshing = true; item->setText(QString::number(it->penaltyAmount, 'f', 2)); m_isRefreshing = false;
            return;
        }
        it->penaltyAmount = sum;
        if (it->penaltyAmount > 100000000.0) {
            it->penaltyAmount = 100000000.0;
            m_isRefreshing = true;
            item->setText(QString::number(it->penaltyAmount, 'f', 2));
            m_isRefreshing = false;
        }
        recalcPenalty = true;
        break;
    }
    case 11: {
        double rate = newVal.toDouble();
        if (!validateRate(rate, errorMsg)) {
            QMessageBox::warning(this, "Ошибка валидации", errorMsg);
            m_isRefreshing = true; item->setText(QString::number(it->penaltyPercent, 'f', 2)); m_isRefreshing = false;
            return;
        }
        // Сначала проверяем, не обнуляем ли оба штрафа
        if (qFuzzyIsNull(rate) && qFuzzyIsNull(it->penaltyAmount)) {
            QMessageBox::warning(this, "Ошибка валидации", "Введите либо фиксированный штраф, либо процентный.");
            m_isRefreshing = true; item->setText(QString::number(it->penaltyPercent, 'f', 2)); m_isRefreshing = false;
            return;
        }
        it->penaltyPercent = rate;
        recalcPenalty = true;
        break;
    }
    case 12:
        return; // вычисляемая колонка, не редактируется
    default: return;
    }

    bool sortingWasEnabled = table->isSortingEnabled();
    table->setSortingEnabled(false);

    if (recalcEndSum) {
        if (it->startSum <= 0 || it->termDays <= 0) {
            it->endSum = 0.0;
        } else {
            double effectiveRate = it->ratePercent;
            if (it->rateType == 1) {
                if (it->startSum >= 500000) effectiveRate += 2.0;
                else if (it->startSum >= 200000) effectiveRate += 1.5;
                else if (it->startSum >= 100000) effectiveRate += 1.0;
                else if (it->startSum >= 50000) effectiveRate += 0.5;
            } else if (it->rateType == 2) {
                if (it->termDays >= 730) effectiveRate += 2.0;
                else if (it->termDays >= 365) effectiveRate += 1.5;
                else if (it->termDays >= 180) effectiveRate += 1.0;
                else if (it->termDays >= 90) effectiveRate += 0.5;
            }

            int periodsPerYear = (it->paymentPeriod == 2) ? 1 : (it->paymentPeriod == 1) ? 4 : 12;
            double periodRate = effectiveRate / periodsPerYear / 100.0;
            double totalPeriods = it->termDays * periodsPerYear / 365.0;
            int nPeriods = qCeil(totalPeriods);
            if (nPeriods < 1) nPeriods = 1;

            if (it->creditType == 0) {
                it->endSum = CreditCalculator::calculateSimple(it->startSum, effectiveRate, it->termDays);
            } else if (it->creditType == 1) {
                double payment = CreditCalculator::calculateAnnuityPayment(it->startSum, periodRate, nPeriods);
                it->endSum = payment * nPeriods;
            } else if (it->creditType == 2) {
                double total = 0.0;
                for (int p = 1; p <= nPeriods; ++p) {
                    total += CreditCalculator::calculateDiffPayment(it->startSum, periodRate, p, nPeriods);
                }
                it->endSum = total;
            }
        }
        if (QTableWidgetItem* cell = table->item(row, 9)) {
            cell->setText(QString::number(it->endSum, 'f', 2));
        }
    }

    if (recalcEndSum || recalcPenalty) {
        QDate issue = QDate::fromString(it->issueDate, "dd.MM.yyyy");
        QDate today = QDate::currentDate();
        double penaltyAmt = 0.0;
        bool overdue = false;
        if (issue.isValid()) {
            QDate due = issue.addDays(it->termDays);
            overdue = (due < today) && (it->endSum > 0);
            if (overdue) {
                int daysLate = due.daysTo(today);
                if (daysLate > 0) {
                    penaltyAmt = CreditCalculator::calculatePenalty(it->endSum, it->penaltyPercent, it->penaltyAmount, daysLate, it->paymentPeriod);
                    if (penaltyAmt > 100000000.0) penaltyAmt = 100000000.0;
                }
            }
        }
        // Обновляем ВЫЧИСЛЯЕМУЮ колонку 12, а не исходные данные 10/11
        if (QTableWidgetItem* cell = table->item(row, 12)) {
            // Вычитаем уже уплаченный штраф, если он есть
            double displayedPenalty = qMax(0.0, penaltyAmt - it->paidPenalty);
            cell->setText(QString::number(displayedPenalty, 'f', 2));
            if (overdue)
                cell->setBackground(QColor(255, 235, 238));
            else
                cell->setBackground(QColor(255, 255, 255));
        }
        bool repaid = qFuzzyIsNull(it->endSum);
        setRowBackgroundStatus(table, row, overdue, repaid);
    }

    table->setSortingEnabled(sortingWasEnabled);
    emit recordUpdated(*it);
}

void CreditTableWidget::onEarlyRepayClicked() {
    int row = table->currentRow();
    if (row < 0) { QMessageBox::warning(this, tr("Внимание"), tr("Выберите запись.")); return; }
    int id = table->item(row, 0)->text().toInt();

    bool ok = false;
    double amount = QInputDialog::getDouble(this, tr("Досрочное погашение"), tr("Сумма:"),
                                            0.0, 0.0, 1e9, 2, &ok);
    if (!ok || amount <= 0) return;

    auto it = std::find_if(records.begin(), records.end(),
                           [id](const CreditRecord& r){ return r.id == id; });
    if (it == records.end()) return;

    QDate today = QDate::currentDate();
    QDate issue = QDate::fromString(it->issueDate, "dd.MM.yyyy");
    QDate due = issue.isValid() ? issue.addDays(it->termDays) : QDate();
    bool overdue = due.isValid() && due < today && it->endSum > 0;
    bool allowed = false;
    if (overdue) {
        allowed = true;
    } else if (it->earlyRepay) {
        allowed = true;
    } else {
        int periodDays = 30;
        if (it->paymentPeriod == static_cast<int>(PaymentPeriod::Quarterly)) periodDays = 90;
        else if (it->paymentPeriod == static_cast<int>(PaymentPeriod::Yearly)) periodDays = 365;
        if (issue.isValid()) {
            int daysSinceIssue = issue.daysTo(today);
            if (daysSinceIssue > 0 && daysSinceIssue % periodDays == 0) {
                allowed = true;
            }
        }
    }
    if (!allowed) {
        QMessageBox::warning(this, tr("Внимание"),
                             tr("Досрочное погашение доступно только в установленные даты (по графику) или при просрочке."));
        return;
    }

    double accruedPenalty = 0.0;
    if (overdue) {
        int daysLate = due.daysTo(today);
        if (daysLate > 0) {
            accruedPenalty = CreditCalculator::calculatePenalty(
                it->endSum, it->penaltyPercent, it->penaltyAmount, daysLate, it->paymentPeriod);
        }
    }

    double remaining = amount;

    // Сначала гасим начисленный штраф и учитываем уже уплаченный
    if (accruedPenalty > 0) {
        // Сумма, которую можно направить на штраф из текущего платежа
        double toPayPenalty = qMin(accruedPenalty, remaining);
        it->paidPenalty += toPayPenalty; // учитываем, что часть штрафа уже погашена
        remaining -= toPayPenalty;
        // Если платеж полностью покрывает штраф, оставшаяся часть будет идти на основной долг
    }

    // Остаток — на погашение основного долга
    it->endSum = qMax(0.0, it->endSum - remaining);

    onSearchClicked();
    emit recordUpdated(*it);
}

QString CreditTableWidget::rateTypeName(int t) {
    switch(t) { case 1: return "От суммы"; case 2: return "От срока"; default: return "Фиксированная"; }
}

QString CreditTableWidget::periodName(int p) {
    switch(p) { case 1: return "Ежеквартально"; case 2: return "Ежегодно"; default: return "Ежемесячно"; }
}

void CreditTableWidget::onExportReport() {
    QList<int> selectedIds;
    if (table->selectionModel()) {
        QModelIndexList indexes = table->selectionModel()->selectedRows();
        for (const QModelIndex &idx : indexes) {
            int row = idx.row();
            if (QTableWidgetItem* idItem = table->item(row, 0)) {
                selectedIds.append(idItem->text().toInt());
            }
        }
    }
    if (selectedIds.isEmpty()) {
        QMessageBox::warning(this, tr("Внимание"), tr("Выберите хотя бы одну запись для экспорта отчёта."));
        return;
    }

    QList<CreditRecord> reportRecords;
    for (int sid : selectedIds) {
        auto it = std::find_if(records.begin(), records.end(),
                               [sid](const CreditRecord& r) { return r.id == sid; });
        if (it != records.end()) {
            reportRecords.append(*it);
        }
    }

    QString clientName = "all_clients";
    if (!reportRecords.isEmpty()) {
        clientName = reportRecords.first().fullName;
        clientName.replace(" ", "_");
    }
    QString defaultName = QDir(QDir::homePath()).filePath(
        QString("отчёт_займы_%1_%2.txt")
            .arg(clientName)
            .arg(QDate::currentDate().toString("dd-MM-yyyy")));

    QString path = QFileDialog::getSaveFileName(
        this, "Сохранить отчёт", defaultName, "Текстовые файлы (*.txt)");
    if (path.isEmpty()) return;

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Ошибка", "Не удалось открыть файл для записи.");
        return;
    }

    QTextStream out(&file);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    out.setEncoding(QStringConverter::Utf8);
#else
    out.setCodec("UTF-8");
#endif

    out << "=== ОТЧЁТ ПО ЗАЙМАМ ===\n";
    out << QString("Дата формирования: %1\n").arg(QDate::currentDate().toString("dd.MM.yyyy"));
    out << QString("Всего записей: %1\n\n").arg(reportRecords.size());

    int overdue = 0;
    QDate today = QDate::currentDate();
    for (const auto& r : reportRecords) {
        QDate d = QDate::fromString(r.issueDate, "dd.MM.yyyy");
        bool isOverdue = d.isValid() && (d.addDays(r.termDays) < today) && (r.endSum > 0);
        if (isOverdue) ++overdue;

        out << QString("ID: %1\n").arg(r.id);
        out << QString("  ФИО:        %1\n").arg(r.fullName);
        out << QString("  Телефон:    %1\n").arg(r.phone);
        out << QString("  Выдан:      %1\n").arg(r.issueDate);
        out << QString("  Срок:       %1 дн.\n").arg(r.termDays);
        out << QString("  Ставка:     %1%\n").arg(r.ratePercent, 0, 'f', 2);
        out << QString("  Нач. сумма: %1 руб.\n").arg(r.startSum, 0, 'f', 2);
        out << QString("  Кон. сумма: %1 руб.\n").arg(r.endSum, 0, 'f', 2);
        if (isOverdue) {
            int daysLate = QDate::fromString(r.issueDate, "dd.MM.yyyy").addDays(r.termDays).daysTo(today);
            double penaltyAmt = (daysLate > 0) ? CreditCalculator::calculatePenalty(r.endSum, r.penaltyPercent, r.penaltyAmount, daysLate, r.paymentPeriod) : 0.0;
            out << QString("  Начислено штрафа: %1 руб.\n").arg(penaltyAmt, 0, 'f', 2);
        }
        out << QString("  Статус:     %1\n").arg(isOverdue ? "ПРОСРОЧЕН" : "активен");
        out << "\n";
    }

    out << "========================\n";
    out << QString("Просроченных: %1 из %2\n").arg(overdue).arg(reportRecords.size());

    file.close();
    QMessageBox::information(this, "Готово",
                             QString("Отчёт сохранён:\n%1").arg(path));
}