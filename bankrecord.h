#ifndef BANKRECORD_H
#define BANKRECORD_H

#include <QString>
#include <QDate>

enum class OperationType { Credit, Deposit };
enum class RateType      { Fixed = 0, DependsOnSum = 1, DependsOnTerm = 2 };
enum class PaymentPeriod { Monthly = 0, Quarterly = 1, Yearly = 2 };
enum class CreditType    { Simple = 0, Annuity = 1, Differentiated = 2 };   // <-- NEW

struct BankRecord {
    int           id            = 0;
    OperationType type          = OperationType::Credit;
    QString       fullName;
    QString       phone;
    QDate         issueDate;
    int           termDays      = 0;
    RateType      rateType      = RateType::Fixed;
    double        ratePercent   = 0.0;
    PaymentPeriod paymentPeriod = PaymentPeriod::Monthly;
    double        startSum      = 0.0;
    double        endSum        = 0.0;
    bool          isEarlyRepaymentAllowed = true;
    double        penaltyAmount  = 0.0;
    double        penaltyPercent = 0.0;
    CreditType    creditType     = CreditType::Simple;

    BankRecord() = default;
    QStringList toCsvRow() const;
    static BankRecord fromCsvRow(const QStringList& row);
};

#endif // BANKRECORD_H
