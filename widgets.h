#pragma once

#include <QFrame>
#include <QPushButton>
#include <QString>

class QLabel;

class ClickableCard : public QFrame
{
    Q_OBJECT
public:
    ClickableCard(const QString &title, const QString &description, QWidget *parent = nullptr);

signals:
    void clicked();

protected:
    void mouseReleaseEvent(QMouseEvent *event) override;
};

class StatCard : public QFrame
{
    Q_OBJECT
public:
    explicit StatCard(QWidget *parent = nullptr);
    void setContent(const QString &title, int value);

private:
    QLabel *m_title;
    QLabel *m_value;
};

class PageHeader : public QWidget
{
    Q_OBJECT
public:
    PageHeader(const QString &crumb, const QString &titleLead, const QString &titleAccent,
               const QString &subtitle, QWidget *parent = nullptr);

signals:
    void homeRequested();
};

QPushButton *makeButton(const QString &text, const QString &background,
                        const QString &foreground = QStringLiteral("#000000"),
                        QWidget *parent = nullptr);

bool confirmDialog(QWidget *parent, const QString &title, const QString &text,
                   const QString &confirmLabel, bool destructive = true);
void infoDialog(QWidget *parent, const QString &title, const QString &text);
