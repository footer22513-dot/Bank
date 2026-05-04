#ifndef DATAHANDLER_H
#define DATAHANDLER_H

#include <QList>
#include <QString>
#include "bankrecord.h"

class DataHandler {
public:
    static bool saveRecords(const QList<BankRecord>& records, const QString& filePath = "records.csv");
    static QList<BankRecord> loadRecords(const QString& filePath = "records.csv");
    static QList<BankRecord> filterRecords(
        const QList<BankRecord>& source,
        const QString& nameFilter,
        double minSum, double maxSum,
        const QDate& dateFrom, const QDate& dateTo,
        OperationType typeFilter = OperationType::Credit
        );
    static int generateNextId(const QList<BankRecord>& records);
};

#endif // DATAHANDLER_H
