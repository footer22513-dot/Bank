#include "depositcalc.h"
#include <cmath>

double DepositCalculator::calculateWithCapitalization(double startSum, double ratePercent, int days) {
    // Сложные проценты: ежедневная капитализация (упрощённо)
    double dailyRate = ratePercent / 36500.0;
    return startSum * pow(1 + dailyRate, days);
}

double DepositCalculator::calculateWithoutCapitalization(double startSum, double ratePercent, int days) {
    // Простые проценты
    return startSum * (1.0 + ratePercent * days / 36500.0);
}

double DepositCalculator::addContribution(double currentSum, double contribution, double ratePercent, int remainingDays) {
    // Добавление взноса с перерасчётом процентов на остаток срока
    double newSum = currentSum + contribution;
    return calculateWithCapitalization(newSum, ratePercent, remainingDays);
}
