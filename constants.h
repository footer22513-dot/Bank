#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QString>

namespace Config {
const QString DATA_FILE = "records.csv";
const QString REPORTS_DIR = "reports/";

// Минимальные/максимальные значения
const double MIN_SUM = 1000.0;
const double MAX_SUM = 100000000.0;
const double MIN_RATE = 0.1;
const double MAX_RATE = 100.0;
const int MIN_TERM_DAYS = 1;
const int MAX_TERM_DAYS = 3650;
}

#endif // CONSTANTS_H
