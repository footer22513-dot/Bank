#include "bankrecord.h"



QStringList BankRecord::toCsvRow() const {
    return {
        QString::number(id),
        (type == OperationType::Credit ? "Credit" : "Deposit"),
        fullName, phone,
        issueDate.toString("yyyy-MM-dd"),
        QString::number(termDays),
        QString::number(static_cast<int>(rateType)),
        QString::number(ratePercent, 'f', 2),
        QString::number(static_cast<int>(paymentPeriod)),
        QString::number(startSum, 'f', 2),
        QString::number(endSum, 'f', 2),
        isEarlyRepaymentAllowed ? "1" : "0",
        QString::number(penaltyAmount, 'f', 2),
        QString::number(penaltyPercent, 'f', 2),
        QString::number(static_cast<int>(creditType))
    };
}

BankRecord BankRecord::fromCsvRow(const QStringList& row) {
    BankRecord r;
    if (row.size() < 14) return r;
    r.id          = row[0].toInt();
    r.type        = (row[1] == "Credit") ? OperationType::Credit : OperationType::Deposit;
    r.fullName    = row[2];
    r.phone       = row[3];
    r.issueDate   = QDate::fromString(row[4], "yyyy-MM-dd");
    r.termDays    = row[5].toInt();
    r.rateType    = static_cast<RateType>(row[6].toInt());
    r.ratePercent = row[7].toDouble();
    r.paymentPeriod = static_cast<PaymentPeriod>(row[8].toInt());
    r.startSum    = row[9].toDouble();
    r.endSum      = row[10].toDouble();
    r.isEarlyRepaymentAllowed = (row[11] == "1");
    r.penaltyAmount  = row[12].toDouble();
    r.penaltyPercent = row[13].toDouble();

    if (row.size() >= 15)                                      // <-- backward compatibility
        r.creditType = static_cast<CreditType>(row[14].toInt());
    else
        r.creditType = CreditType::Simple;

    return r;
}
