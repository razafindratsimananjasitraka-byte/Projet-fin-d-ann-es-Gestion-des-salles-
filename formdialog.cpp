#include "formdialog.h"

#include "theme.h"

#include <QComboBox>
#include <QDate>
#include <QDateEdit>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTime>
#include <QTimeEdit>
#include <QVBoxLayout>

namespace {

QFrame *makeSeparator(QWidget *parent)
{
    auto *line = new QFrame(parent);
    line->setFixedHeight(1);
    line->setStyleSheet(QStringLiteral("background: #33425B; border: none;"));
    return line;
}

}

FormDialog::FormDialog(const QString &titleLead, const QString &titleAccent, const QString &subtitle,
                       QWidget *parent)
    : QDialog(parent)
{
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setModal(true);
    setMinimumWidth(560);

    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(12, 12, 12, 12);

    auto *card = new QFrame(this);
    card->setObjectName(QStringLiteral("dialogCard"));
    card->setStyleSheet(QStringLiteral(
        "QFrame#dialogCard { background: %1; border: 1px solid %2; border-radius: 18px; }"
        "QLabel { background: transparent; color: #FFFFFF; }")
                            .arg(Theme::kPanel, Theme::kBorder));
    outer->addWidget(card);

    auto *root = new QVBoxLayout(card);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    auto *header = new QWidget(card);
    header->setStyleSheet(QStringLiteral("background: transparent;"));
    auto *headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(28, 24, 20, 18);

    auto *titles = new QVBoxLayout;
    titles->setSpacing(4);
    auto *titleLabel = new QLabel(header);
    titleLabel->setTextFormat(Qt::RichText);
    titleLabel->setText(QStringLiteral(
        "<span style=\"font-size:24px; font-weight:700;\">%1<span style=\"color:%2;\">%3</span></span>")
                            .arg(titleLead, Theme::kAccent, titleAccent));
    auto *subtitleLabel = new QLabel(subtitle, header);
    subtitleLabel->setStyleSheet(QStringLiteral("font-size: 13px; color: #FFFFFF;"));
    titles->addWidget(titleLabel);
    titles->addWidget(subtitleLabel);
    headerLayout->addLayout(titles, 1);

    auto *close = new QPushButton(QStringLiteral("×"), header);
    close->setCursor(Qt::PointingHandCursor);
    close->setFixedSize(34, 34);
    close->setStyleSheet(QStringLiteral(
        "QPushButton { background: transparent; color: #FFFFFF; border: none; font-size: 24px; }"
        "QPushButton:hover { color: #FFD500; }"));
    connect(close, &QPushButton::clicked, this, &QDialog::reject);
    headerLayout->addWidget(close, 0, Qt::AlignTop);

    root->addWidget(header);
    root->addWidget(makeSeparator(card));

    auto *body = new QWidget(card);
    body->setStyleSheet(QStringLiteral("background: transparent;"));
    m_grid = new QGridLayout(body);
    m_grid->setContentsMargins(28, 20, 28, 12);
    m_grid->setHorizontalSpacing(20);
    m_grid->setVerticalSpacing(14);
    m_grid->setColumnMinimumWidth(0, 160);
    m_grid->setColumnStretch(1, 1);
    root->addWidget(body);

    m_error = new QLabel(card);
    m_error->setWordWrap(true);
    m_error->setStyleSheet(QStringLiteral("color: #FF5C5C; font-size: 13px; padding: 0 28px 8px 28px;"));
    m_error->hide();
    root->addWidget(m_error);

    root->addWidget(makeSeparator(card));

    auto *footer = new QWidget(card);
    footer->setStyleSheet(QStringLiteral("background: transparent;"));
    auto *footerLayout = new QHBoxLayout(footer);
    footerLayout->setContentsMargins(28, 16, 28, 20);
    footerLayout->setSpacing(12);
    footerLayout->addStretch();

    m_cancel = new QPushButton(QStringLiteral("Annuler"), footer);
    m_cancel->setCursor(Qt::PointingHandCursor);
    m_cancel->setAutoDefault(false);
    m_cancel->setStyleSheet(QStringLiteral(
        "QPushButton { background: transparent; color: #FFFFFF; border: 1px solid #4A5A78;"
        " border-radius: 10px; padding: 10px 24px; font-weight: 600; }"
        "QPushButton:hover { border: 1px solid #FFD500; }"));
    connect(m_cancel, &QPushButton::clicked, this, &QDialog::reject);

    m_save = new QPushButton(QStringLiteral("Enregistrer"), footer);
    m_save->setCursor(Qt::PointingHandCursor);
    m_save->setDefault(true);
    m_save->setStyleSheet(QStringLiteral(
        "QPushButton { background: #FFD500; color: #000000; border: 1px solid #FFD500;"
        " border-radius: 10px; padding: 10px 28px; font-weight: 700; }"
        "QPushButton:hover { border: 1px solid #FFFFFF; }"));
    connect(m_save, &QPushButton::clicked, this, &FormDialog::accept);

    footerLayout->addWidget(m_cancel);
    footerLayout->addWidget(m_save);
    root->addWidget(footer);
}

void FormDialog::addRow(const QString &label, QWidget *field, bool required)
{
    auto *labelWidget = new QLabel(this);
    labelWidget->setTextFormat(Qt::RichText);
    labelWidget->setText(required
                             ? QStringLiteral("%1 <span style=\"color:#FF5C5C;\">*</span>").arg(label)
                             : label);
    labelWidget->setStyleSheet(QStringLiteral("font-size: 14px; font-weight: 600; background: transparent;"));

    m_grid->addWidget(labelWidget, m_row, 0);
    m_grid->addWidget(field, m_row, 1);
    m_fields.append(field);
    ++m_row;
}

QLineEdit *FormDialog::addText(const QString &label, const QString &placeholder, bool required)
{
    auto *edit = new QLineEdit(this);
    edit->setPlaceholderText(placeholder);
    addRow(label, edit, required);
    return edit;
}

QSpinBox *FormDialog::addNumber(const QString &label, int minimum, int maximum, int value, bool required)
{
    auto *spin = new QSpinBox(this);
    spin->setRange(minimum, maximum);
    spin->setValue(value);
    spin->setButtonSymbols(QAbstractSpinBox::NoButtons);
    addRow(label, spin, required);
    return spin;
}

QComboBox *FormDialog::addCombo(const QString &label, const QStringList &items, bool required)
{
    auto *combo = new QComboBox(this);
    combo->addItems(items);
    addRow(label, combo, required);
    return combo;
}

QDateEdit *FormDialog::addDate(const QString &label, const QDate &value, bool required)
{
    auto *edit = new QDateEdit(value, this);
    edit->setDisplayFormat(QStringLiteral("dd/MM/yyyy"));
    edit->setCalendarPopup(true);
    addRow(label, edit, required);
    return edit;
}

QTimeEdit *FormDialog::addTime(const QString &label, const QTime &value, bool required)
{
    auto *edit = new QTimeEdit(value, this);
    edit->setDisplayFormat(QStringLiteral("HH:mm"));
    edit->setButtonSymbols(QAbstractSpinBox::NoButtons);
    addRow(label, edit, required);
    return edit;
}

void FormDialog::setValidator(std::function<QString()> validator)
{
    m_validator = std::move(validator);
}

void FormDialog::setReadOnly(bool readOnly)
{
    if (!readOnly) {
        return;
    }
    for (QWidget *field : m_fields) {
        if (auto *line = qobject_cast<QLineEdit *>(field)) {
            line->setReadOnly(true);
        } else if (auto *spin = qobject_cast<QAbstractSpinBox *>(field)) {
            spin->setReadOnly(true);
        } else {
            field->setEnabled(false);
        }
    }
    m_save->hide();
    m_cancel->setText(QStringLiteral("Fermer"));
}

void FormDialog::accept()
{
    if (m_validator) {
        const QString message = m_validator();
        if (!message.isEmpty()) {
            m_error->setText(message);
            m_error->show();
            return;
        }
    }
    QDialog::accept();
}
