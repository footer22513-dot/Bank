#include "depositwidget.h"
#include <QTableWidgetItem>
#include <QDate>
#include <QtWidgets>
#include <algorithm>
#include <cmath>
#include "depositcalc.h"
#include <QRegularExpression>

// ── Валидация ─────────────────────────────────────────────

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
    if (phone.length() < 5) {
        error = "Телефон слишком короткий";
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

static bool validateTermDays(int days, QString& error) {
    if (days <= 0) {
        error = "Срок должен быть положительным числом";
        return false;
    }
    if (days > 36500) {
        error = "Срок не может превышать 100 лет (36500 дней)";
        return false;
    }
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

// ── Классы сортировки ─────────────────────────────────────

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

// ── Конструктор ───────────────────────────────────────────

DepositTableWidget::DepositTableWidget(QWidget *parent) : BaseWidget(parent) {
    auto* main = new QHBoxLayout(this);

    table = new QTableWidget;
    table->setColumnCount(11);
    table->setHorizontalHeaderLabels({
        "ID", "ФИО", "Телефон", "Дата начала",
        "Срок (дн.)", "Тип ставки", "Ставка (%)",
        "Периодичность", "Нач. сумма", "Накоплено", "Кон. сумма"
    });
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    table->horizontalHeader()->setStretchLastSection(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
    table->setSortingEnabled(true);
    main->addWidget(table, 3);

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

    ctrl->addWidget(new QLabel("Дата начала от:"));
    dateFrom = new QDateEdit(QDate(2000,1,1)); dateFrom->setCalendarPopup(true);
    dateFrom->setDisplayFormat("dd.MM.yyyy");
    ctrl->addWidget(dateFrom);
    ctrl->addWidget(new QLabel("Дата начала до:"));
    dateTo = new QDateEdit(QDate::currentDate().addYears(10)); dateTo->setCalendarPopup(true);
    dateTo->setDisplayFormat("dd.MM.yyyy");
    ctrl->addWidget(dateTo);

    searchBtn = new QPushButton("🔍 ПОИСК");
    searchBtn->setStyleSheet("font-size:14px;padding:8px;background:#1976D2;color:white;border-radius:5px;");
    connect(searchBtn, &QPushButton::clicked, this, &DepositTableWidget::onSearchClicked);
    ctrl->addWidget(searchBtn);

    resetBtn = new QPushButton("СБРОСИТЬ");
    resetBtn->setStyleSheet("font-size:14px;padding:6px;background:#78909C;color:white;border-radius:5px;");
    connect(resetBtn, &QPushButton::clicked, this, &DepositTableWidget::onResetClicked);
    ctrl->addWidget(resetBtn);

    ctrl->addStretch();

    delBtn = new QPushButton("🗑 УДАЛИТЬ");
    delBtn->setStyleSheet("font-size:14px;padding:8px;background:#f44336;color:white;border-radius:5px;");
    connect(delBtn, &QPushButton::clicked, this, &DepositTableWidget::onDeleteClicked);
    ctrl->addWidget(delBtn);

    topUpBtn = new QPushButton("💰 Пополнить");
    topUpBtn->setStyleSheet("font-size:14px;padding:8px;background:#388E3C;color:white;border-radius:5px;");
    connect(topUpBtn, &QPushButton::clicked, this, &DepositTableWidget::onTopUpClicked);
    ctrl->addWidget(topUpBtn);

    withdrawBtn = new QPushButton("💸 Снять");
    withdrawBtn->setStyleSheet("font-size:14px;padding:8px;background:#D32F2F;color:white;border-radius:5px;");
    connect(withdrawBtn, &QPushButton::clicked, this, &DepositTableWidget::onWithdrawClicked);
    ctrl->addWidget(withdrawBtn);

    backBtn = new QPushButton("← НАЗАД");
    backBtn->setStyleSheet("font-size:14px;padding:8px;background:#555;color:white;border-radius:5px;");
    connect(backBtn, &QPushButton::clicked, this, &DepositTableWidget::navigateToAdmin);
    ctrl->addWidget(backBtn);

    exportBtn = new QPushButton("📄 Экспорт отчёта");
    exportBtn->setStyleSheet("font-size:14px;padding:8px;background:#1976D2;color:white;border-radius:5px;");
    connect(exportBtn, &QPushButton::clicked, this, &DepositTableWidget::onExportReport);
    ctrl->addWidget(exportBtn);

    connect(table, &QTableWidget::itemChanged, this, &DepositTableWidget::onItemChanged);

    // Таймер обновления накопленных процентов — с parent для автоматического удаления
    QTimer* updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, [this]() {
        if (!m_isRefreshing) refreshTable(records);
    });
    updateTimer->start(30000);

    setLayout(main);
}

// ── Работа с данными ──────────────────────────────────────

void DepositTableWidget::setRecords(const QList<DepositRecord>& data) {
    records = data;
    refreshTable(records);
}

QList<DepositRecord> DepositTableWidget::getRecords() const {
    return records;
}

// ── Пересчёт накопленных процентов ──────────────────────

static double calculateAccrued(const DepositRecord& r) {
    QDate startDate = QDate::fromString(r.issueDate, "dd.MM.yyyy");
    if (!startDate.isValid()) return r.startSum;

    int daysElapsed = startDate.daysTo(QDate::currentDate());
    if (daysElapsed < 0) return r.startSum;
    if (daysElapsed >= r.termDays) return r.endSum;

    int periodsPerYear = (r.paymentPeriod == 2) ? 1 : (r.paymentPeriod == 1) ? 4 : 12;
    double periodRate = r.ratePercent / periodsPerYear / 100.0;
    double elapsedPeriods = daysElapsed * periodsPerYear / 365.0;
    return r.startSum * pow(1.0 + periodRate, elapsedPeriods);
}

// ── Обновление таблицы ──────────────────────────────────

void DepositTableWidget::refreshTable(const QList<DepositRecord>& data) {
    m_isRefreshing = true;
    table->setSortingEnabled(false);
    table->setRowCount(0);

    for (const auto& r : data) {
        int row = table->rowCount();
        table->insertRow(row);

        // ID (нередактируемый)
        QTableWidgetItem* idItem = new QTableWidgetItem(QString::number(r.id));
        idItem->setFlags(idItem->flags() & ~Qt::ItemIsEditable);
        table->setItem(row, 0, idItem);

        // ФИО, телефон
        table->setItem(row, 1, new QTableWidgetItem(r.fullName));
        table->setItem(row, 2, new QTableWidgetItem(r.phone));

        // Дата
        table->setItem(row, 3, new DateTableItem(r.issueDate));

        // Срок
        table->setItem(row, 4, new NumericTableItem(QString::number(r.termDays)));

        // Тип ставки (нередактируемый)
        QTableWidgetItem* rateTypeItem = new QTableWidgetItem(rateTypeName(r.rateType));
        rateTypeItem->setFlags(rateTypeItem->flags() & ~Qt::ItemIsEditable);
        table->setItem(row, 5, rateTypeItem);

        // Ставка
        table->setItem(row, 6, new NumericTableItem(QString::number(r.ratePercent, 'f', 2)));

        // Периодичность (нередактируемый) — БЫЛО ПРОПУЩЕНО!
        QTableWidgetItem* periodItem = new QTableWidgetItem(periodName(r.paymentPeriod));
        periodItem->setFlags(periodItem->flags() & ~Qt::ItemIsEditable);
        table->setItem(row, 7, periodItem);

        // Начальная сумма
        table->setItem(row, 8, new NumericTableItem(QString::number(r.startSum, 'f', 2)));

        // Накоплено (вычисляемое, нередактируемое)
        double accrued = calculateAccrued(r);
        QTableWidgetItem* accruedItem = new NumericTableItem(QString::number(accrued, 'f', 2));
        accruedItem->setFlags(accruedItem->flags() & ~Qt::ItemIsEditable);
        accruedItem->setBackground(QColor(232, 245, 233));
        table->setItem(row, 9, accruedItem);

        // Конечная сумма
        table->setItem(row, 10, new NumericTableItem(QString::number(r.endSum, 'f', 2)));
    }

    table->setSortingEnabled(true);
    m_isRefreshing = false;
}

// ── Поиск и фильтры ───────────────────────────────────────

void DepositTableWidget::onSearchClicked() {
    QString f = searchInput->text().trimmed().toLower();
    double minS = sumMin->value(), maxS = sumMax->value();
    QDate df = dateFrom->date(), dt = dateTo->date();

    QList<DepositRecord> out;
    for (const auto& r : records) {
        QDate d = QDate::fromString(r.issueDate, "dd.MM.yyyy");
        bool nameOk = f.isEmpty() || r.fullName.toLower().contains(f)
                      || r.phone.contains(f)
                      || QString::number(r.id).contains(f);
        bool sumOk = r.startSum >= minS && r.startSum <= maxS;
        bool dateOk = (!d.isValid()) || (d >= df && d <= dt);
        if (nameOk && sumOk && dateOk) out.append(r);
    }
    refreshTable(out);
}

void DepositTableWidget::onResetClicked() {
    searchInput->clear();
    sumMin->setValue(0); sumMax->setValue(999999999);
    dateFrom->setDate(QDate(2000,1,1));
    dateTo->setDate(QDate::currentDate().addYears(10));
    refreshTable(records);
}

// ── Удаление (исправлено: множественное + корректная логика) ──

void DepositTableWidget::onDeleteClicked() {
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

    // Собираем ID до удаления
    QList<int> idsToDelete;
    for (const QModelIndex &idx : selected) {
        int row = idx.row();
        if (QTableWidgetItem* idItem = table->item(row, 0)) {
            idsToDelete.append(idItem->text().toInt());
        }
    }

    // Удаляем из records
    for (int id : idsToDelete) {
        records.erase(std::remove_if(records.begin(), records.end(),
                                     [id](const DepositRecord& r){ return r.id == id; }), records.end());
        emit recordDeleted(id);
    }

    onSearchClicked();
}

// ── Редактирование ячеек (исправлено: защита от nullptr) ──

void DepositTableWidget::onItemChanged(QTableWidgetItem* item) {
    if (m_isRefreshing) return;
    if (!item) return;

    int row = item->row();
    int col = item->column();
    if (row < 0 || row >= table->rowCount()) return;

    // Защита: ID должен существовать
    QTableWidgetItem* idItem = table->item(row, 0);
    if (!idItem) return;

    int id = idItem->text().toInt();
    auto it = std::find_if(records.begin(), records.end(),
                           [id](const DepositRecord& r) { return r.id == id; });
    if (it == records.end()) return;

    QString newVal = item->text();
    QString errorMsg;
    bool needsRecalculate = false;

    switch(col) {
    case 0: // ID — нередактируемый, игнорируем
        return;
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
        needsRecalculate = true;
        break;
    case 4: {
        int days = newVal.toInt();
        if (!validateTermDays(days, errorMsg)) {
            QMessageBox::warning(this, "Ошибка валидации", errorMsg);
            m_isRefreshing = true; item->setText(QString::number(it->termDays)); m_isRefreshing = false;
            return;
        }
        it->termDays = days;
        needsRecalculate = true;
        break;
    }
    case 5: // Тип ставки — нередактируемый
        return;
    case 6: {
        double rate = newVal.toDouble();
        if (!validateRate(rate, errorMsg)) {
            QMessageBox::warning(this, "Ошибка валидации", errorMsg);
            m_isRefreshing = true; item->setText(QString::number(it->ratePercent, 'f', 2)); m_isRefreshing = false;
            return;
        }
        it->ratePercent = rate;
        needsRecalculate = true;
        break;
    }
    case 7: // Периодичность — нередактируемый
        return;
    case 8: {
        double sum = newVal.toDouble();
        if (!validateSum(sum, errorMsg)) {
            QMessageBox::warning(this, "Ошибка валидации", errorMsg);
            m_isRefreshing = true; item->setText(QString::number(it->startSum, 'f', 2)); m_isRefreshing = false;
            return;
        }
        it->startSum = sum;
        needsRecalculate = true;
        break;
    }
    case 9: // Накоплено — вычисляемое, нередактируемое
        return;
    case 10: {
        double sum = newVal.toDouble();
        if (!validateSum(sum, errorMsg)) {
            QMessageBox::warning(this, "Ошибка валидации", errorMsg);
            m_isRefreshing = true; item->setText(QString::number(it->endSum, 'f', 2)); m_isRefreshing = false;
            return;
        }
        it->endSum = sum;
        break;
    }
    default:
        return;
    }

    // Пересчёт конечной суммы и накопленных процентов
    if (needsRecalculate && it->startSum > 0 && it->termDays > 0) {
        double effectiveRate = it->ratePercent;
        if (it->rateType == 1) { // от суммы
            if (it->startSum >= 100000) effectiveRate += 1.5;
            else if (it->startSum >= 50000) effectiveRate += 1.0;
            else if (it->startSum >= 10000) effectiveRate += 0.5;
        } else if (it->rateType == 2) { // от срока
            if (it->termDays >= 730) effectiveRate += 1.5;
            else if (it->termDays >= 365) effectiveRate += 1.0;
            else if (it->termDays >= 180) effectiveRate += 0.5;
        }

        int periodsPerYear = (it->paymentPeriod == 2) ? 1 : (it->paymentPeriod == 1) ? 4 : 12;
        double periodRate = effectiveRate / periodsPerYear / 100.0;
        double totalPeriods = it->termDays * periodsPerYear / 365.0;
        it->endSum = it->startSum * pow(1.0 + periodRate, totalPeriods);

        // Обновляем конечную сумму
        if (QTableWidgetItem* cell = table->item(row, 10)) {
            cell->setText(QString::number(it->endSum, 'f', 2));
        }

        // Обновляем накоплено
        double accrued = calculateAccrued(*it);
        if (QTableWidgetItem* cell = table->item(row, 9)) {
            cell->setText(QString::number(accrued, 'f', 2));
        }
    }

    emit recordUpdated(*it);
}

// ── Вспомогательные функции ───────────────────────────────

QString DepositTableWidget::rateTypeName(int t) {
    switch(t) { case 1: return "От суммы"; case 2: return "От срока"; default: return "Фиксированная"; }
}

QString DepositTableWidget::periodName(int p) {
    switch(p) { case 1: return "Ежеквартально"; case 2: return "Ежегодно"; default: return "Ежемесячно"; }
}

// ── Операции с вкладами ─────────────────────────────────

void DepositTableWidget::onTopUpClicked() {
    int row = table->currentRow();
    if (row < 0) { QMessageBox::warning(this, tr("Внимание"), tr("Выберите запись.")); return; }

    QTableWidgetItem* idItem = table->item(row, 0);
    if (!idItem) return;

    int id = idItem->text().toInt();
    bool ok = false;
    double amount = QInputDialog::getDouble(this, tr("Пополнение вклада"), tr("Сумма:"),
                                            0.0, 0.0, 1e9, 2, &ok);
    if (!ok || amount <= 0) return;

    auto it = std::find_if(records.begin(), records.end(),
                           [id](const DepositRecord& r){ return r.id == id; });
    if (it == records.end()) return;

    it->startSum = qMax(0.0, it->startSum + amount);
    it->endSum = qMax(0.0, DepositCalculator::calculateWithCapitalization(it->startSum, it->ratePercent, it->termDays));
    refreshTable(records);
    emit recordUpdated(*it);
}

void DepositTableWidget::onWithdrawClicked() {
    int row = table->currentRow();
    if (row < 0) { QMessageBox::warning(this, tr("Внимание"), tr("Выберите запись.")); return; }

    QTableWidgetItem* idItem = table->item(row, 0);
    if (!idItem) return;

    int id = idItem->text().toInt();
    auto it = std::find_if(records.begin(), records.end(),
                           [id](const DepositRecord& r){ return r.id == id; });
    if (it == records.end()) return;

    bool ok = false;
    double amount = QInputDialog::getDouble(this, tr("Снятие части вклада"), tr("Сумма:"),
                                            0.0, 0.0, it->startSum, 2, &ok);
    if (!ok || amount <= 0) return;

    if (amount > it->startSum) {
        QMessageBox::warning(this, tr("Ошибка"), tr("Сумма снятия превышает текущую сумму вклада."));
        return;
    }

    it->startSum = qMax(0.0, it->startSum - amount);
    it->endSum = qMax(0.0, DepositCalculator::calculateWithCapitalization(it->startSum, it->ratePercent, it->termDays));
    refreshTable(records);
    emit recordUpdated(*it);
}

// ── Экспорт (исправлено: работа с ID вместо индексов таблицы) ──

void DepositTableWidget::onExportReport() {
    // Собираем ID выделенных записей
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

    // Формируем список записей для отчёта
    QList<DepositRecord> reportRecords;
    if (!selectedIds.isEmpty()) {
        for (int sid : selectedIds) {
            auto it = std::find_if(records.begin(), records.end(),
                                   [sid](const DepositRecord& r) { return r.id == sid; });
            if (it != records.end()) reportRecords.append(*it);
        }
    } else {
        reportRecords = records;
    }

    if (reportRecords.isEmpty()) {
        QMessageBox::warning(this, tr("Внимание"), tr("Нет данных для экспорта."));
        return;
    }

    QString clientName = reportRecords.first().fullName;
    clientName.replace(" ", "_");
    QString defaultName = QDir::homePath() + QString("/отчёт_вклады_%1_%2.txt")
                                                 .arg(clientName)
                                                 .arg(QDate::currentDate().toString("dd-MM-yyyy"));

    QString path = QFileDialog::getSaveFileName(this, "Сохранить отчёт", defaultName, "Текстовые файлы (*.txt)");
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

    out << "=== ОТЧЁТ ПО ВКЛАДАМ ===\n";
    out << QString("Дата формирования: %1\n").arg(QDate::currentDate().toString("dd.MM.yyyy"));
    out << QString("Всего записей: %1\n\n").arg(reportRecords.size());

    QDate today = QDate::currentDate();
    int overdue = 0;
    for (const auto& r : reportRecords) {
        QDate d = QDate::fromString(r.issueDate, "dd.MM.yyyy");
        QDate due = d.isValid() ? d.addDays(r.termDays) : QDate();
        bool isOverdue = d.isValid() && (due < today) && (r.endSum > 0);
        if (isOverdue) ++overdue;

        out << QString("ID: %1\n").arg(r.id);
        out << QString("  ФИО:        %1\n").arg(r.fullName);
        out << QString("  Телефон:    %1\n").arg(r.phone);
        out << QString("  Открыт:     %1\n").arg(r.issueDate);
        out << QString("  Срок:       %1 дн.\n").arg(r.termDays);
        out << QString("  Дата конца: %1\n").arg(due.isValid() ? due.toString("dd.MM.yyyy") : "—");
        out << QString("  Ставка:     %1%\n").arg(r.ratePercent, 0, 'f', 2);
        out << QString("  Нач. сумма: %1 руб.\n").arg(r.startSum, 0, 'f', 2);
        out << QString("  Кон. сумма: %1 руб.\n").arg(r.endSum, 0, 'f', 2);
        out << QString("  Накоплено: %1 руб.\n").arg(calculateAccrued(r), 0, 'f', 2);
        out << QString("  Статус:     %1\n").arg(isOverdue ? "ПРОСРОЧЕН" : "активен");
        out << "\n";
    }

    out << "========================\n";
    out << QString("Просроченных: %1 из %2\n").arg(overdue).arg(reportRecords.size());
    file.close();
    QMessageBox::information(this, "Готово", QString("Отчёт сохранён:\n%1").arg(path));
}