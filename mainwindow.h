#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QList>
#include <QDate>

#include "piechartwidget.h"
#include "bankrecord.h"
#include "credittablewidget.h"
#include "depositwidget.h"
#include "barchartwidget.h"
#include "linechartwidget.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class LoginWidget;
class AdminWidget;
class UserWidget;
class CreditFormWidget;
class DepositFormWidget;
class SuccessWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    Ui::MainWindow*     ui;
    QStackedWidget*     stack;

    LoginWidget*        loginWgt;
    AdminWidget*        adminWgt;
    UserWidget*         userWgt;
    CreditTableWidget*  creditTableWgt;
    DepositTableWidget* depositTableWgt;
    CreditFormWidget*   creditFormWgt;
    DepositFormWidget*  depositFormWgt;
    SuccessWidget*      successWgt;

    PieChartWidget*     pieChartWgt;
    QWidget*            analyticsContainer;  // ← добавлено

    BarChartWidget*     barChartWgt;
    LineChartWidget*    lineChartWgt;
    QStackedWidget*     analyticsStack;

    QPushButton*        pieBtn;
    QPushButton*        barBtn;
    QPushButton*        lineBtn;

    QList<BankRecord> allRecords;
    QString           currentFile;

    QList<CreditRecord>  toCreditList()  const;
    QList<DepositRecord> toDepositList() const;

    void loadFile(const QString& path);
    void saveFile(const QString& path);

private slots:
    void goLogin();
    void goAdmin();
    void goUser();
    void goCreditTable();
    void goDepositTable();
    void goCreditForm();
    void goDepositForm();
    void goSuccess();
    void goPieChart();

    void onCreditReady(const QString& fio, const QString& phone,
                       QDate issueDate, int days,
                       int rateType, double rate, int period,
                       double startSum, double endSum, bool earlyRepay,
                       double penaltyAmount, double penaltyPercent,
                       int creditType);

    void onDepositReady(const QString& fio, const QString& phone,
                        QDate issueDate, int days,
                        int rateType, double rate, int period,
                        double startSum, double endSum);

    void onCreditDeleted(int id);
    void onDepositDeleted(int id);

    void onCreditUpdated(const CreditRecord& rec);
    void onDepositUpdated(const DepositRecord& rec);

    void onLoadDb();
    void onSaveDb();
};

#endif // MAINWINDOW_H
