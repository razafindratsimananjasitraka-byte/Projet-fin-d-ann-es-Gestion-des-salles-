#pragma once

#include <QMainWindow>
#include <QVector>

class QLineEdit;
class QPushButton;
class QStackedWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    void showPage(int index);

    QStackedWidget *m_stack;
    QVector<QPushButton *> m_navButtons;
    QWidget *m_searchBar;
    QLineEdit *m_search;
};
