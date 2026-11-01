#pragma once

#include <QStringList>
#include <QVector>
#include <QWidget>

class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QTableWidget;
class StatCard;

struct RowData
{
    int id = 0;
    QStringList cells;
};

struct PageConfig
{
    QString crumb;
    QString titleLead;
    QString titleAccent;
    QString subtitle;
    QString addLabel;
    QStringList statLabels;  // exactly three entries
    QStringList headers;     // columns shown between the index and the actions
    QStringList filters;     // entries offered after "Tous"
    int statusColumn = -1;   // index inside RowData::cells holding the status
};

class BasePage : public QWidget
{
    Q_OBJECT
public:
    explicit BasePage(const PageConfig &config, QWidget *parent = nullptr);

    void reload();
    void setSearchText(const QString &text);

signals:
    void homeRequested();

protected:
    virtual QVector<RowData> loadRows() = 0;
    virtual QVector<int> loadStats() = 0;
    virtual void addRecord() = 0;
    virtual void editRecord(int id) = 0;
    virtual void viewRecord(int id) = 0;
    virtual bool deleteRecord(int id, QString *error) = 0;
    virtual QString deleteMessage() const = 0;

private:
    void applyFilters();
    void render();
    QWidget *buildActions(int id);
    void requestDelete(int id);

    PageConfig m_config;
    QVector<RowData> m_all;
    QVector<RowData> m_filtered;
    int m_page = 0;

    StatCard *m_stats[3];
    QLineEdit *m_search;
    QComboBox *m_filter;
    QTableWidget *m_table;
    QPushButton *m_previous;
    QPushButton *m_next;
    QLabel *m_pageLabel;
};
