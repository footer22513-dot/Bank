#include "creditcalc.h"
#include <QDate>
#include <cmath>

double CreditCalculator::calculateSimple(double startSum, double ratePercent, int days) {
    return startSum * (1.0 + ratePercent * days / 36500.0);
}

double CreditCalculator::calculateAnnuityPayment(double principal, double monthlyRate, int months) {
    if (monthlyRate == 0) return principal / months;
    double factor = pow(1 + monthlyRate, months);
    return principal * monthlyRate * factor / (factor - 1);
}

double CreditCalculator::calculateDiffPayment(double principal, double monthlyRate, int month, int totalMonths) {
    double principalPart = principal / totalMonths;
    double interestPart = (principal - principalPart * (month - 1)) * monthlyRate;
    return principalPart + interestPart;
}

double CreditCalculator::calculatePenalty(double remainingSum, double penaltyPercent, double penaltyAmount, int daysLate, int paymentPeriod) {
    if (daysLate <= 0) return 0.0;

    int periodDays = 30;      // Monthly
    int periodsPerYear = 12;
    if (paymentPeriod == 1) { // Quarterly
        periodDays = 90;
        periodsPerYear = 4;
    } else if (paymentPeriod == 2) { // Yearly
        periodDays = 365;
        periodsPerYear = 1;
    }

    // Количество просроченных периодов (округление вверх)
    int periodsLate = (daysLate + periodDays - 1) / periodDays;

    double penalty = 0.0;
    if (penaltyAmount > 0.0) {
        penalty += penaltyAmount * periodsLate;
    }
    if (penaltyPercent > 0.0) {
        penalty += remainingSum * penaltyPercent * periodsLate / (100.0 * periodsPerYear);
    }
    return penalty;
}

double CreditCalculator::recalculateAfterEarlyPayment(BankRecord& record, double paymentAmount) {
    if (paymentAmount >= record.endSum) {
        record.endSum = 0;
        return 0;
    }
    record.endSum -= paymentAmount;
    return record.endSum;
}