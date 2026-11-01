#include "widgets.h"

#include "theme.h"

#include <QCursor>
#include <QLabel>
#include <QMessageBox>
#include <QMouseEvent>
#include <QVBoxLayout>

ClickableCard::ClickableCard(const QString &title, const QString &description, QWidget *parent)
    : QFrame(parent)
{
    setObjectName(QStringLiteral("clickableCard"));
    setCursor(Qt::PointingHandCursor);
    setMinimumHeight(130);
    setStyleSheet(QStringLiteral(
        "QFrame#clickableCard { background: %1; border: 1px solid %2; border-radius: 16px; }"
        "QFrame#clickableCard:hover { background: #26344D; border: 1px solid %3; }")
                      .arg(Theme::kPanel, Theme::kBorder, Theme::kAccent));

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(8);

    auto *titleLabel = new QLabel(QStringLiteral("<u>%1</u>").arg(title), this);
    titleLabel->setTextFormat(Qt::RichText);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(QStringLiteral("font-size: 19px; color: #FFFFFF; background: transparent;"));
    titleLabel->setAttribute(Qt::WA_TransparentForMouseEvents);

    auto *descriptionLabel = new QLabel(description, this);
    descriptionLabel->setAlignment(Qt::AlignCenter);
    descriptionLabel->setWordWrap(true);
    descriptionLabel->setStyleSheet(
        QStringLiteral("font-size: 12px; color: %1; background: transparent;").arg(Theme::kMuted));
    descriptionLabel->setAttribute(Qt::WA_TransparentForMouseEvents);

    layout->addStretch();
    layout->addWidget(titleLabel);
    layout->addWidget(descriptionLabel);
    layout->addStretch();
}

void ClickableCard::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && rect().contains(mapFromGlobal(QCursor::pos()))) {
        emit clicked();
    }
    QFrame::mouseReleaseEvent(event);
}

StatCard::StatCard(QWidget *parent)
    : QFrame(parent)
{
    setObjectName(QStringLiteral("statCard"));
    setMinimumHeight(96);
    setStyleSheet(QStringLiteral(
        "QFrame#statCard { background: %1; border: 1px solid %2; border-radius: 16px; }")
                      .arg(Theme::kPanel, Theme::kBorder));

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(4);

    m_title = new QLabel(this);
    m_title->setAlignment(Qt::AlignCenter);
    m_title->setStyleSheet(QStringLiteral("font-size: 16px; color: #FFFFFF; background: transparent;"));

    m_value = new QLabel(this);
    m_value->setAlignment(Qt::AlignCenter);
    m_value->setStyleSheet(QStringLiteral(
        "font-size: 30px; font-weight: 700; font-style: italic; color: #FFFFFF; background: transparent;"));

    layout->addWidget(m_title);
    layout->addWidget(m_value);
}

void StatCard::setContent(const QString &title, int value)
{
    m_title->setText(title);
    m_value->setText(QString::number(value));
}

PageHeader::PageHeader(const QString &crumb, const QString &titleLead, const QString &titleAccent,
                       const QString &subtitle, QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);

    auto *crumbLabel = new QLabel(this);
    crumbLabel->setTextFormat(Qt::RichText);
    crumbLabel->setText(QStringLiteral(
        "<a href=\"home\" style=\"text-decoration:none;\"><span style=\"color:#FFFFFF;\">Accueil</span></a>"
        "<span style=\"color:#FFFFFF;\"> &gt; <b>%1</b></span>").arg(crumb));
    crumbLabel->setStyleSheet(QStringLiteral("font-size: 14px; background: transparent;"));
    connect(crumbLabel, &QLabel::linkActivated, this, [this] { emit homeRequested(); });

    auto *titleLabel = new QLabel(this);
    titleLabel->setTextFormat(Qt::RichText);
    titleLabel->setText(QStringLiteral(
        "<span style=\"font-size:34px; font-weight:700; color:#FFFFFF;\">%1"
        "<span style=\"color:%2;\">%3</span></span>")
                            .arg(titleLead, Theme::kAccent, titleAccent));
    titleLabel->setStyleSheet(QStringLiteral("background: transparent;"));

    auto *subtitleLabel = new QLabel(subtitle, this);
    subtitleLabel->setStyleSheet(QStringLiteral("font-size: 13px; color: #FFFFFF; background: transparent;"));

    layout->addWidget(crumbLabel);
    layout->addWidget(titleLabel);
    layout->addWidget(subtitleLabel);
}

QPushButton *makeButton(const QString &text, const QString &background, const QString &foreground,
                        QWidget *parent)
{
    auto *button = new QPushButton(text, parent);
    button->setCursor(Qt::PointingHandCursor);
    button->setStyleSheet(QStringLiteral(
        "QPushButton { background: %1; color: %2; border: 1px solid %1; border-radius: 8px;"
        " padding: 6px 14px; font-weight: 600; }"
        "QPushButton:hover { border: 1px solid #FFFFFF; }"
        "QPushButton:disabled { background: #141C2C; color: #566178; border: 1px solid #141C2C; }")
                              .arg(background, foreground));
    return button;
}

bool confirmDialog(QWidget *parent, const QString &title, const QString &text,
                   const QString &confirmLabel, bool destructive)
{
    QMessageBox box(QMessageBox::NoIcon, title, text, QMessageBox::NoButton, parent);
    QPushButton *confirm = box.addButton(confirmLabel, QMessageBox::AcceptRole);
    QPushButton *cancel = box.addButton(QStringLiteral("Annuler"), QMessageBox::RejectRole);

    const QString base = QStringLiteral(
        "QPushButton { min-width: 90px; padding: 8px 18px; border-radius: 8px; font-weight: 600; %1 }");
    confirm->setStyleSheet(base.arg(destructive
                                        ? QStringLiteral("background: #FF2A2A; color: #FFFFFF; border: none;")
                                        : QStringLiteral("background: #FFD500; color: #000000; border: none;")));
    cancel->setStyleSheet(base.arg(QStringLiteral("background: transparent; color: #FFFFFF; border: 1px solid #4A5A78;")));
    box.setDefaultButton(cancel);
    box.exec();
    return box.clickedButton() == confirm;
}

void infoDialog(QWidget *parent, const QString &title, const QString &text)
{
    QMessageBox box(QMessageBox::NoIcon, title, text, QMessageBox::NoButton, parent);
    QPushButton *ok = box.addButton(QStringLiteral("Fermer"), QMessageBox::AcceptRole);
    ok->setStyleSheet(QStringLiteral(
        "QPushButton { min-width: 90px; padding: 8px 18px; border-radius: 8px; font-weight: 600;"
        " background: #FFD500; color: #000000; border: none; }"));
    box.exec();
}
