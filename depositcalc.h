#ifndef DEPOSITCALC_H
#define DEPOSITCALC_H

#include "bankrecord.h"

class DepositCalculator {
public:
    static double calculateWithCapitalization(double startSum, double ratePercent, int days);
    static double calculateWithoutCapitalization(double startSum, double ratePercent, int days);
    static double addContribution(double currentSum, double contribution, double ratePercent, int remainingDays);
};

#endif // DEPOSITCALC_H
