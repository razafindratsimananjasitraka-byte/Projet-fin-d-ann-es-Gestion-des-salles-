#pragma once

#include <QDate>
#include <QWidget>

class QComboBox;
class QLabel;
class QTableWidget;
class StatCard;

class PlanningPage : public QWidget
{
    Q_OBJECT
public:
    explicit PlanningPage(QWidget *parent = nullptr);

    void reload();

signals:
    void homeRequested();

private:
    void shiftWeek(int days);
    void fill();

    QDate m_weekStart;
    QLabel *m_weekLabel;
    QComboBox *m_roomFilter;
    QTableWidget *m_grid;
    StatCard *m_stats[3];
};
