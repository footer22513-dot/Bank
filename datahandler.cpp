#include "datahandler.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>

bool DataHandler::saveRecords(const QList<BankRecord>& records, const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Error saving:" << file.errorString();
        return false;
    }

    QTextStream out(&file);
    // Заголовок (опционально)
    // out << "ID;Type;FullName;Phone;IssueDate;TermDays;RateType;RatePercent;PaymentPeriod;StartSum;EndSum;EarlyRepay;PenaltyAmt;PenaltyPct\n";

    for (const auto& r : records) {
        out << r.toCsvRow().join(";") << "\n";
    }

    file.close();
    return true;
}

QList<BankRecord> DataHandler::loadRecords(const QString& filePath) {
    QList<BankRecord> records;
    QFile file(filePath);

    if (!file.exists() || !file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return records; // Пустой список, если файл не найден
    }

    QTextStream in(&file);
    // Пропускаем заголовок, если есть
    // in.readLine();

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;

        auto row = line.split(";");
        if (row.size() >= 14) {
            records.append(BankRecord::fromCsvRow(row));
        }
    }

    file.close();
    return records;
}

QList<BankRecord> DataHandler::filterRecords(
    const QList<BankRecord>& source,
    const QString& nameFilter,
    double minSum, double maxSum,
    const QDate& dateFrom, const QDate& dateTo,
    OperationType typeFilter)
{
    QList<BankRecord> result;
    for (const auto& r : source) {
        if (r.type != typeFilter) continue;

        bool matchName = r.fullName.contains(nameFilter, Qt::CaseInsensitive);
        bool matchSum = r.startSum >= minSum && r.startSum <= maxSum;
        bool matchDate = r.issueDate >= dateFrom && r.issueDate <= dateTo;

        if (matchName && matchSum && matchDate) {
            result.append(r);
        }
    }
    return result;
}

int DataHandler::generateNextId(const QList<BankRecord>& records) {
    int maxId = 0;
    for (const auto& r : records) {
        if (r.id > maxId) maxId = r.id;
    }
    return maxId + 1;
}
