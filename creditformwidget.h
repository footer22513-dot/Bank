#ifndef CREDITFORMWIDGET_H
#define CREDITFORMWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QDateEdit>
#include <QCheckBox>

class CreditFormWidget : public QWidget {
    Q_OBJECT
public:
    explicit CreditFormWidget(QWidget *parent = nullptr);

signals:
    void navigateToUser();
    void navigateToSuccess();
    void creditDataReady(const QString& fio, const QString& phone,
                         QDate issueDate, int days,
                         int rateType, double rate, int period,
                         double startSum, double endSum,
                         bool earlyRepay,
                         double penaltyAmount, double penaltyPercent,
                         int creditType);

private slots:
    void calculateEndSum();
    void onCreateCreditClicked();

private:
    QLineEdit*  fioInput;
    QLineEdit*  phoneInput;
    QDateEdit*  issueDateEdit;   // ТЗ п.3.2.1: дата выдачи
    QComboBox*  termCombo;       // ТЗ п.3.1.3: предустановленные сроки
    QLineEdit*  daysInput;       // произвольный срок
    QComboBox*  rateTypeCombo;
    QComboBox*  rateCombo;       // фиксированные ставки из ТЗ
    QComboBox*  periodCombo;
    QLineEdit*  startSumInput;
    QCheckBox*  earlyRepayCheck; // ТЗ п.3.2.1: досрочная выплата
    QLabel*     endSumLabel;
    QLineEdit*   penaltyAmountInput;
    QLineEdit*   penaltyPercentInput;
    QComboBox*   creditTypeCombo;
    QPushButton* createBtn;
    QPushButton* backBtn;

    int    selectedDays() const;
    double selectedRate() const;
    void   setupUI();
    void   setupConnections();
};

#endif // CREDITFORMWIDGET_H
