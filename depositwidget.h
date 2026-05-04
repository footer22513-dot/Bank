#ifndef DEPOSITTABLEWIDGET_H
#define DEPOSITTABLEWIDGET_H

#include "basewidget.h"
#include <QList>
#include <QDate>

// Структура по ТЗ п.3.2.1
struct DepositRecord {
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
};

class QTableWidget;
class QLineEdit;
class QSpinBox;
class QDateEdit;
class QPushButton;
class QLabel;
class QTableWidgetItem;

class DepositTableWidget : public BaseWidget {
    Q_OBJECT
public:
    explicit DepositTableWidget(QWidget *parent = nullptr);

    void setRecords(const QList<DepositRecord>& records);
    QList<DepositRecord> getRecords() const;

signals:
    void recordDeleted(int id);
    void recordUpdated(const DepositRecord& record);  // Для сохранения изменений

private slots:
    void onSearchClicked();
    void onResetClicked();
    void onDeleteClicked();
    void onItemChanged(QTableWidgetItem* item);       // Обработчик редактирования
    void onTopUpClicked();
    void onWithdrawClicked();
    void onExportReport(); // export overdue deposits report    // Пополнение вклада

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
    QPushButton*  topUpBtn; // Кнопка "Пополнить"
    QPushButton*  withdrawBtn; // Кнопка "Снять"
    QPushButton*  backBtn;
    QPushButton*  exportBtn; // button to export TXT report

    QList<DepositRecord> records;
    bool m_isRefreshing = false;                       // Флаг блокировки сигналов

    void refreshTable(const QList<DepositRecord>& data);
    static QString rateTypeName(int t);
    static QString periodName(int p);
};

#endif // DEPOSITTABLEWIDGET_H
