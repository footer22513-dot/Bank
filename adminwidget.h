#ifndef ADMINWIDGET_H
#define ADMINWIDGET_H

#include "basewidget.h"
#include <QList>

class QPushButton;
class QLabel;
struct BankRecord;

class AdminWidget : public BaseWidget {
    Q_OBJECT
public:
    explicit AdminWidget(QWidget *parent = nullptr);
    void setRecords(const QList<BankRecord>* records);

signals:
    void loadDatabaseRequested();
    void saveDatabaseRequested();

private slots:
    void onExportAnalyticsClicked();

private:
    QPushButton* analytics;
    QPushButton* creditTableBtn;
    QPushButton* depositTableBtn;
    QPushButton* loadDbBtn;
    QPushButton* saveDbBtn;
    QPushButton* backBtn;
    QPushButton* exportBtn;
    QLabel*      titleLabel;
    const QList<BankRecord>* m_records = nullptr;
};

#endif // ADMINWIDGET_H