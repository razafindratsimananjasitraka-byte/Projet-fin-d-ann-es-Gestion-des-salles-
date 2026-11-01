#pragma once

#include "basepage.h"

class MatierePage : public BasePage
{
    Q_OBJECT
public:
    explicit MatierePage(QWidget *parent = nullptr);

protected:
    QVector<RowData> loadRows() override;
    QVector<int> loadStats() override;
    void addRecord() override;
    void editRecord(int id) override;
    void viewRecord(int id) override;
    bool deleteRecord(int id, QString *error) override;
    QString deleteMessage() const override;

private:
    void showForm(int id, bool readOnly);
};
