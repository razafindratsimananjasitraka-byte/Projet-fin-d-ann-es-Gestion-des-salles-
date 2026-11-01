#pragma once

#include <QDialog>
#include <QList>
#include <functional>

class QComboBox;
class QDate;
class QDateEdit;
class QGridLayout;
class QLabel;
class QLineEdit;
class QPushButton;
class QSpinBox;
class QTime;
class QTimeEdit;

class FormDialog : public QDialog
{
    Q_OBJECT
public:
    FormDialog(const QString &titleLead, const QString &titleAccent, const QString &subtitle,
               QWidget *parent = nullptr);

    QLineEdit *addText(const QString &label, const QString &placeholder, bool required = false);
    QSpinBox *addNumber(const QString &label, int minimum, int maximum, int value, bool required = false);
    QComboBox *addCombo(const QString &label, const QStringList &items, bool required = false);
    QDateEdit *addDate(const QString &label, const QDate &value, bool required = false);
    QTimeEdit *addTime(const QString &label, const QTime &value, bool required = false);

    // The validator returns an error message, or an empty string when the form is valid.
    void setValidator(std::function<QString()> validator);
    void setReadOnly(bool readOnly);

public slots:
    void accept() override;

private:
    void addRow(const QString &label, QWidget *field, bool required);

    QGridLayout *m_grid;
    QLabel *m_error;
    QPushButton *m_save;
    QPushButton *m_cancel;
    QList<QWidget *> m_fields;
    std::function<QString()> m_validator;
    int m_row = 0;
};
