#ifndef DEPOSITFORMWIDGET_H
#define DEPOSITFORMWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QDateEdit>

class DepositFormWidget : public QWidget {
    Q_OBJECT
public:
    explicit DepositFormWidget(QWidget *parent = nullptr);

signals:
    void exportAnalyticsRequested();
    void navigateToUser();
    void navigateToSuccess();
    void depositDataReady(const QString& fio, const QString& phone,
                          QDate issueDate, int days,
                          int rateType, double rate, int period,
                          double startSum, double endSum);

private slots:
    void calculateEndSum();
    void onCreateDepositClicked();
    void onExportAnalyticsClicked();

private:
    QLineEdit*   fioInput;
    QLineEdit*   phoneInput;
    QDateEdit*   issueDateEdit;
    QComboBox*   termCombo;
    QLineEdit*   daysInput;
    QComboBox*   rateTypeCombo;
    QComboBox*   rateCombo;      // фиксированные ставки
    QComboBox*   periodCombo;
    QLineEdit*   startSumInput;
    QLabel*      endSumLabel;
    QLineEdit*   penaltyAmountInput;
    QLineEdit*   penaltyPercentInput;
    QPushButton* createBtn;
    QPushButton* exportAnalyticsBtn;
    QPushButton* backBtn;

    int    selectedDays() const;
    double selectedRate() const;
    void   setupUI();
    void   setupConnections();
};

#endif // DEPOSITFORMWIDGET_H
