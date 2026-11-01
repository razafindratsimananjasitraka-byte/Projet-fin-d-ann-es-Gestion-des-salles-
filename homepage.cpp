#include "homepage.h"

#include "theme.h"
#include "widgets.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

HomePage::HomePage(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(40, 32, 40, 32);
    layout->setSpacing(8);

    auto *welcome = new QLabel(QStringLiteral("Bienvenue sur"), this);
    welcome->setStyleSheet(QStringLiteral("font-size: 18px; color: #FFFFFF;"));

    auto *title = new QLabel(this);
    title->setTextFormat(Qt::RichText);
    title->setText(QStringLiteral(
        "<span style=\"font-size:44px; font-weight:700; color:#FFFFFF;\">Gestion des "
        "<span style=\"color:%1;\">Salles</span></span>").arg(Theme::kAccent));

    auto *tagline = new QLabel(QStringLiteral("Réservez ● Gérez ● Organisez facilement vos salles"), this);
    tagline->setStyleSheet(QStringLiteral("font-size: 13px; color: #FFFFFF;"));

    layout->addWidget(welcome);
    layout->addWidget(title);
    layout->addWidget(tagline);
    layout->addSpacing(28);

    struct Entry
    {
        QString title;
        QString description;
        int page;
    };
    const Entry entries[] = {
        {QStringLiteral("Salles"), QStringLiteral("Consultez les salles disponibles"), 1},
        {QStringLiteral("Enseignants"), QStringLiteral("Gérez les informations des enseignants"), 2},
        {QStringLiteral("Matières"), QStringLiteral("Organisez les matières enseignées"), 3},
        {QStringLiteral("Réservations"), QStringLiteral("Gérez les réservations des salles"), 4}};

    auto *row = new QHBoxLayout;
    row->setSpacing(18);
    for (const Entry &entry : entries) {
        auto *card = new ClickableCard(entry.title, entry.description, this);
        const int page = entry.page;
        connect(card, &ClickableCard::clicked, this, [this, page] { emit pageRequested(page); });
        row->addWidget(card);
    }
    layout->addLayout(row);
    layout->addStretch();
}
