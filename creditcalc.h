#ifndef CREDITCALC_H
#define CREDITCALC_H

#include "bankrecord.h"

class CreditCalculator {
public:
    static double calculateSimple(double startSum, double ratePercent, int days);
    static double calculateAnnuityPayment(double principal, double monthlyRate, int months);
    static double calculateDiffPayment(double principal, double monthlyRate, int month, int totalMonths);
    // начисляет штраф за каждый просроченный период (месяц/квартал/год)
    static double calculatePenalty(double remainingSum, double penaltyPercent, double penaltyAmount, int daysLate, int paymentPeriod);
    static double recalculateAfterEarlyPayment(BankRecord& record, double paymentAmount);
};

#endif // CREDITCALC_H