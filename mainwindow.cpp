#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include "loginwidget.h"
#include "adminwidget.h"
#include "userwidget.h"
#include "credittablewidget.h"
#include "depositwidget.h"
#include "creditformwidget.h"
#include "depositformwidget.h"
#include "successwidget.h"
#include "datahandler.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <QFileInfo>
#include <QLayout>
#include <QLabel>
#include <QFrame>
#include <QPushButton>
#include <QVBoxLayout>
#include "piechartwidget.h"

static const QString DEFAULT_FILE = "bank_records.txt";

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow),
    currentFile(DEFAULT_FILE)
{
    ui->setupUi(this);
    resize(1280, 800);
    setMinimumSize(1024, 768);
    setWindowTitle("АРМ «Займы и Вклады»");

    // ── Создаём виджеты ────────────────────────────────────
    loginWgt        = new LoginWidget(this);
    adminWgt        = new AdminWidget(this);
    adminWgt->setRecords(&allRecords);
    userWgt         = new UserWidget(this);
    creditTableWgt  = new CreditTableWidget(this);
    depositTableWgt = new DepositTableWidget(this);
    creditFormWgt   = new CreditFormWidget(this);
    depositFormWgt  = new DepositFormWidget(this);
    successWgt      = new SuccessWidget(this);
    pieChartWgt     = new PieChartWidget(this);

    // ── Контейнер аналитики (FIX #1) ───────────────────────
    // pieChartWgt нельзя добавлять и в стек, и в layout одновременно.
    // Решение: создаём отдельный контейнер, кладём в него pieChartWgt и кнопку.
    // --- Виджеты аналитики ---
    barChartWgt  = new BarChartWidget(this);
    lineChartWgt = new LineChartWidget(this);

    analyticsStack = new QStackedWidget(this);
    analyticsStack->addWidget(pieChartWgt);
    analyticsStack->addWidget(barChartWgt);
    analyticsStack->addWidget(lineChartWgt);

    auto* btnLayout = new QHBoxLayout;
    btnLayout->setSpacing(10);
    pieBtn = new QPushButton("📊 Соотношение");
    barBtn = new QPushButton("📈 По ставкам");
    lineBtn = new QPushButton("📉 Динамика");
    for (auto* b : {pieBtn, barBtn, lineBtn}) {
        b->setMinimumHeight(40);
        b->setStyleSheet("font-size:14px; background:#6A1B9A; color:white; border-radius:6px;");
        btnLayout->addWidget(b);
    }
    connect(pieBtn,  &QPushButton::clicked, [this](){ analyticsStack->setCurrentIndex(0); });
    connect(barBtn,  &QPushButton::clicked, [this](){ analyticsStack->setCurrentIndex(1); });
    connect(lineBtn, &QPushButton::clicked, [this](){ analyticsStack->setCurrentIndex(2); });

    // --- Контейнер аналитики ---
    analyticsContainer = new QWidget(this);
    auto* analyticsLayout = new QVBoxLayout(analyticsContainer);
    analyticsLayout->setContentsMargins(32, 32, 32, 24);
    analyticsLayout->setSpacing(16);

    QLabel* chartTitle = new QLabel("АНАЛИТИКА");
    chartTitle->setAlignment(Qt::AlignCenter);
    chartTitle->setStyleSheet("font-size:32px; font-weight:bold;");
    analyticsLayout->addWidget(chartTitle);

    QFrame* divider = new QFrame; divider->setFrameShape(QFrame::HLine);
    analyticsLayout->addWidget(divider);
    analyticsLayout->addLayout(btnLayout);
    analyticsLayout->addWidget(analyticsStack, 1);

    QPushButton* backFromAnalytics = new QPushButton("← НАЗАД");
    backFromAnalytics->setMinimumHeight(50);
    backFromAnalytics->setStyleSheet("font-size:16px; background:#555; color:white; border-radius:6px;");
    connect(backFromAnalytics, &QPushButton::clicked, this, &MainWindow::goAdmin);
    analyticsLayout->addWidget(backFromAnalytics);

    // ── Стек ───────────────────────────────────────────────
    stack = new QStackedWidget(this);
    setCentralWidget(stack);

    stack->addWidget(loginWgt);
    stack->addWidget(adminWgt);
    stack->addWidget(userWgt);
    stack->addWidget(creditTableWgt);
    stack->addWidget(depositTableWgt);
    stack->addWidget(creditFormWgt);
    stack->addWidget(depositFormWgt);
    stack->addWidget(successWgt);
    stack->addWidget(analyticsContainer);  // контейнер, не pieChartWgt напрямую

    // ── Навигация ──────────────────────────────────────────
    connect(loginWgt, &LoginWidget::navigateToAdmin, this, &MainWindow::goAdmin);
    connect(loginWgt, &LoginWidget::navigateToUser,  this, &MainWindow::goUser);

    connect(adminWgt, &AdminWidget::navigateToCreditTable,  this, &MainWindow::goCreditTable);
    connect(adminWgt, &AdminWidget::navigateToDepositTable, this, &MainWindow::goDepositTable);
    connect(adminWgt, &AdminWidget::navigateToLogin,        this, &MainWindow::goLogin);
    connect(adminWgt, &AdminWidget::loadDatabaseRequested,  this, &MainWindow::onLoadDb);
    connect(adminWgt, &AdminWidget::saveDatabaseRequested,  this, &MainWindow::onSaveDb);
    connect(adminWgt, &AdminWidget::navigateToAnalytics,    this, &MainWindow::goPieChart);

    connect(userWgt, &UserWidget::navigateToCreditForm,  this, &MainWindow::goCreditForm);
    connect(userWgt, &UserWidget::navigateToDepositForm, this, &MainWindow::goDepositForm);
    connect(userWgt, &UserWidget::navigateToLogin,       this, &MainWindow::goLogin);

    connect(creditTableWgt,  &CreditTableWidget::navigateToAdmin,  this, &MainWindow::goAdmin);
    connect(depositTableWgt, &DepositTableWidget::navigateToAdmin, this, &MainWindow::goAdmin);
    connect(creditTableWgt,  &CreditTableWidget::recordDeleted,    this, &MainWindow::onCreditDeleted);
    connect(depositTableWgt, &DepositTableWidget::recordDeleted,   this, &MainWindow::onDepositDeleted);
    connect(creditTableWgt,  &CreditTableWidget::recordUpdated,    this, &MainWindow::onCreditUpdated);
    connect(depositTableWgt, &DepositTableWidget::recordUpdated,   this, &MainWindow::onDepositUpdated);

    connect(creditFormWgt, &CreditFormWidget::navigateToUser,    this, &MainWindow::goUser);
    connect(creditFormWgt, &CreditFormWidget::navigateToSuccess, this, &MainWindow::goSuccess);
    connect(creditFormWgt, &CreditFormWidget::creditDataReady,   this, &MainWindow::onCreditReady);

    connect(depositFormWgt, &DepositFormWidget::navigateToUser,    this, &MainWindow::goUser);
    connect(depositFormWgt, &DepositFormWidget::navigateToSuccess, this, &MainWindow::goSuccess);
    connect(depositFormWgt, &DepositFormWidget::depositDataReady,  this, &MainWindow::onDepositReady);

    connect(successWgt, &SuccessWidget::navigateToUser, this, &MainWindow::goUser);

    loadFile(currentFile);
    stack->setCurrentWidget(loginWgt);
}

MainWindow::~MainWindow() { delete ui; }

// ── Файлы ──────────────────────────────────────────────────
void MainWindow::loadFile(const QString& path) {
    allRecords = DataHandler::loadRecords(path);
}

void MainWindow::saveFile(const QString& path) {
    DataHandler::saveRecords(allRecords, path);
}

void MainWindow::onLoadDb() {
    QString path = QFileDialog::getOpenFileName(this, "Загрузить базу",
                                                QDir::homePath(), "Текстовые файлы (*.txt);;Все файлы (*)");
    if (path.isEmpty()) return;
    loadFile(path);
    currentFile = path;
    QMessageBox::information(this, "Загрузка",
                             QString("Загружено записей: %1").arg(allRecords.size()));
}

void MainWindow::onSaveDb() {
    QString path = QFileDialog::getSaveFileName(this, "Выгрузить базу",
                                                QDir::homePath() + "/" + QFileInfo(currentFile).fileName(),
                                                "Текстовые файлы (*.txt);;Все файлы (*)");
    if (path.isEmpty()) return;
    saveFile(path);
    currentFile = path;
    QMessageBox::information(this, "Выгрузка",
                             QString("База сохранена:\n%1").arg(path));
}

// ── Конвертеры ─────────────────────────────────────────────
QList<CreditRecord> MainWindow::toCreditList() const {
    QList<CreditRecord> out;
    for (const auto& r : allRecords) {
        if (r.type != OperationType::Credit) continue;
        CreditRecord cr;
        cr.id             = r.id;
        cr.fullName       = r.fullName;
        cr.phone          = r.phone;
        cr.issueDate      = r.issueDate.toString("dd.MM.yyyy");
        cr.termDays       = r.termDays;
        cr.rateType       = static_cast<int>(r.rateType);
        cr.ratePercent    = r.ratePercent;
        cr.paymentPeriod  = static_cast<int>(r.paymentPeriod);
        cr.startSum       = r.startSum;
        cr.endSum         = r.endSum;
        cr.earlyRepay     = r.isEarlyRepaymentAllowed;
        cr.penaltyAmount  = r.penaltyAmount;
        cr.penaltyPercent = r.penaltyPercent;
        cr.creditType     = static_cast<int>(r.creditType);  // FIX #2: поле не копировалось
        out.append(cr);
    }
    return out;
}

QList<DepositRecord> MainWindow::toDepositList() const {
    QList<DepositRecord> out;
    for (const auto& r : allRecords) {
        if (r.type != OperationType::Deposit) continue;
        DepositRecord dr;
        dr.id            = r.id;
        dr.fullName      = r.fullName;
        dr.phone         = r.phone;
        dr.issueDate     = r.issueDate.toString("dd.MM.yyyy");
        dr.termDays      = r.termDays;
        dr.rateType      = static_cast<int>(r.rateType);
        dr.ratePercent   = r.ratePercent;
        dr.paymentPeriod = static_cast<int>(r.paymentPeriod);
        dr.startSum      = r.startSum;
        dr.endSum        = r.endSum;
        out.append(dr);
    }
    return out;
}

// ── Навигация ──────────────────────────────────────────────
void MainWindow::goLogin()       { stack->setCurrentWidget(loginWgt); }
void MainWindow::goAdmin()       { stack->setCurrentWidget(adminWgt); }
void MainWindow::goUser()        { stack->setCurrentWidget(userWgt); }
void MainWindow::goSuccess()     { stack->setCurrentWidget(successWgt); }
void MainWindow::goCreditForm()  { stack->setCurrentWidget(creditFormWgt); }
void MainWindow::goDepositForm() { stack->setCurrentWidget(depositFormWgt); }

void MainWindow::goPieChart() {
    // 1. Pie — количество операций
    int credits = 0, deposits = 0;
    for (const auto& r : allRecords) {
        if (r.type == OperationType::Credit) ++credits;
        else ++deposits;
    }
    pieChartWgt->updateData(credits, deposits);

    // 2. Bar — распределение по типам ставок (суммы)
    QMap<int, double> rateSums;
    for (const auto& r : allRecords)
        rateSums[static_cast<int>(r.rateType)] += r.startSum;

    QVector<BarItem> bars;
    QStringList lbl = {"Фиксированная", "От суммы", "От срока"};
    QColor clr[3] = {QColor("#1976D2"), QColor("#388E3C"), QColor("#F57C00")};
    for (int i = 0; i < 3; ++i)
        if (rateSums.value(i, 0) > 0)
            bars.append({lbl[i], rateSums.value(i, 0), clr[i]});
    barChartWgt->setData(bars, "Распределение по типам ставок (руб.)");

    // 3. Line — динамика по датам
    QMap<QDate, TimePoint> tm;
    for (const auto& r : allRecords) {
        if (!r.issueDate.isValid()) continue;
        if (!tm.contains(r.issueDate)) tm[r.issueDate] = {r.issueDate, 0, 0};
        if (r.type == OperationType::Credit) tm[r.issueDate].creditSum += r.startSum;
        else tm[r.issueDate].depositSum += r.startSum;
    }
    QVector<TimePoint> pts;
    for (auto it = tm.begin(); it != tm.end(); ++it) pts.append(it.value());
    std::sort(pts.begin(), pts.end(),
              [](const TimePoint& a, const TimePoint& b){ return a.date < b.date; });
    lineChartWgt->setData(pts);

    stack->setCurrentWidget(analyticsContainer);
}
void MainWindow::goCreditTable() {
    creditTableWgt->setRecords(toCreditList());
    stack->setCurrentWidget(creditTableWgt);
}

void MainWindow::goDepositTable() {
    depositTableWgt->setRecords(toDepositList());
    stack->setCurrentWidget(depositTableWgt);
}

// ── Приём данных ───────────────────────────────────────────
void MainWindow::onCreditReady(const QString& fio, const QString& phone,
                               QDate issueDate, int days,
                               int rateType, double rate, int period,
                               double startSum, double endSum, bool earlyRepay,
                               double penaltyAmount, double penaltyPercent,
                               int creditType)
{
    BankRecord r;
    r.id                     = DataHandler::generateNextId(allRecords);
    r.type                   = OperationType::Credit;
    r.fullName               = fio;
    r.phone                  = phone;
    r.issueDate              = issueDate;
    r.termDays               = days;
    r.rateType               = static_cast<RateType>(rateType);
    r.ratePercent            = rate;
    r.paymentPeriod          = static_cast<PaymentPeriod>(period);
    r.startSum               = startSum;
    r.endSum                 = endSum;
    r.isEarlyRepaymentAllowed = earlyRepay;
    r.penaltyAmount          = penaltyAmount;
    r.penaltyPercent         = penaltyPercent;
    r.creditType             = static_cast<CreditType>(creditType);
    allRecords.append(r);
    saveFile(currentFile);
}

void MainWindow::onDepositReady(const QString& fio, const QString& phone,
                                QDate issueDate, int days,
                                int rateType, double rate, int period,
                                double startSum, double endSum)
{
    BankRecord r;
    r.id            = DataHandler::generateNextId(allRecords);
    r.type          = OperationType::Deposit;
    r.fullName      = fio;
    r.phone         = phone;
    r.issueDate     = issueDate;
    r.termDays      = days;
    r.rateType      = static_cast<RateType>(rateType);
    r.ratePercent   = rate;
    r.paymentPeriod = static_cast<PaymentPeriod>(period);
    r.startSum      = startSum;
    r.endSum        = endSum;
    allRecords.append(r);
    saveFile(currentFile);
}

void MainWindow::onCreditDeleted(int id) {
    for (auto it = allRecords.begin(); it != allRecords.end(); ) {
        if (it->id == id && it->type == OperationType::Credit)
            it = allRecords.erase(it);
        else
            ++it;
    }
    saveFile(currentFile);
}

void MainWindow::onDepositDeleted(int id) {
    for (auto it = allRecords.begin(); it != allRecords.end(); ) {
        if (it->id == id && it->type == OperationType::Deposit)
            it = allRecords.erase(it);
        else
            ++it;
    }
    saveFile(currentFile);
}

void MainWindow::onCreditUpdated(const CreditRecord& rec) {
    for (auto& r : allRecords) {
        if (r.id == rec.id && r.type == OperationType::Credit) {
            r.fullName       = rec.fullName;
            r.phone          = rec.phone;
            r.issueDate      = QDate::fromString(rec.issueDate, "dd.MM.yyyy");
            r.termDays       = rec.termDays;
            r.ratePercent    = rec.ratePercent;
            r.startSum       = rec.startSum;
            r.endSum         = rec.endSum;
            r.penaltyAmount  = rec.penaltyAmount;
            r.penaltyPercent = rec.penaltyPercent;
            // Additional fields from credit record
            r.rateType       = static_cast<RateType>(rec.rateType);
            r.paymentPeriod  = static_cast<PaymentPeriod>(rec.paymentPeriod);
            r.isEarlyRepaymentAllowed = rec.earlyRepay;
            r.creditType     = static_cast<CreditType>(rec.creditType);
            break;
        }
    }
    saveFile(currentFile);
}

void MainWindow::onDepositUpdated(const DepositRecord& rec) {
    for (auto& r : allRecords) {
        if (r.id == rec.id && r.type == OperationType::Deposit) {
            r.fullName    = rec.fullName;
            r.phone       = rec.phone;
            r.issueDate   = QDate::fromString(rec.issueDate, "dd.MM.yyyy");
            r.termDays    = rec.termDays;
            r.ratePercent = rec.ratePercent;
            r.startSum    = rec.startSum;
            r.endSum      = rec.endSum;
            break;
        }
    }
    saveFile(currentFile);
}