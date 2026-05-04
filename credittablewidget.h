#ifndef CREDITTABLEWIDGET_H
#define CREDITTABLEWIDGET_H
#include "basewidget.h"
#include <QList>
#include <QDate>
#include <QCheckBox>

struct CreditRecord {
    int     id;
    QString fullName;
    QString phone;
    QString issueDate;
    int     termDays;
    int     rateType;
    double  ratePercent;
    int     paymentPeriod;
    double  startSum;
    double  endSum;
    bool    earlyRepay;
    double  penaltyAmount;
    double  penaltyPercent;
    double  paidPenalty = 0.0; // уже уплаченный штраф
    int     creditType = 0;
};

class QTableWidget;
class QLineEdit;
class QSpinBox;
class QDateEdit;
class QPushButton;
class QLabel;
class QTableWidgetItem;

class CreditTableWidget : public BaseWidget {
    Q_OBJECT
public:
    explicit CreditTableWidget(QWidget *parent = nullptr);
    void setRecords(const QList<CreditRecord>& records);
    QList<CreditRecord> getRecords() const;
signals:
    void recordDeleted(int id);
    void recordUpdated(const CreditRecord& record);
private slots:
    void onSearchClicked();
    void onResetClicked();
    void onDeleteClicked();
    void onItemChanged(QTableWidgetItem* item);
    void onEarlyRepayClicked();
    void onExportReport();
private:
    QTableWidget* table;
    QLineEdit*    searchInput;
    QSpinBox*     sumMin;
    QSpinBox*     sumMax;
    QDateEdit*    dateFrom;
    QDateEdit*    dateTo;
    QPushButton*  searchBtn;
    QPushButton*  resetBtn;
    QPushButton*  delBtn;
    QPushButton*  repayBtn;
    QPushButton*  backBtn;
    QPushButton*  exportBtn; // button to export TXT report
    QCheckBox*    overdueOnly;
    QList<CreditRecord> records;
    bool m_isRefreshing = false;
    void refreshTable(const QList<CreditRecord>& data);
    static QString rateTypeName(int t);
    static QString periodName(int p);
};
#endif // CREDITTABLEWIDGET_H