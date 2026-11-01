#include "mainwindow.h"

#include "basepage.h"
#include "enseignantpage.h"
#include "homepage.h"
#include "matierepage.h"
#include "planningpage.h"
#include "reservationpage.h"
#include "sallepage.h"
#include "theme.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(QStringLiteral("Gestion de Salles"));
    resize(1280, 800);
    setMinimumSize(1040, 680);

    auto *central = new QWidget(this);
    central->setObjectName(QStringLiteral("root"));
    central->setStyleSheet(QStringLiteral("QWidget#root { background: %1; }").arg(Theme::kBackground));
    setCentralWidget(central);

    auto *mainLayout = new QHBoxLayout(central);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Sidebar
    auto *sidebar = new QFrame(central);
    sidebar->setObjectName(QStringLiteral("sidebar"));
    sidebar->setFixedWidth(200);
    sidebar->setStyleSheet(QStringLiteral(
        "QFrame#sidebar { background: %1; border-right: 1px solid %2; }"
        "QFrame#sidebar QPushButton { background: #FFFFFF; color: #000000; border: none;"
        " border-radius: 8px; padding: 10px; font-size: 14px; }"
        "QFrame#sidebar QPushButton:hover { background: #E6E6E6; }"
        "QFrame#sidebar QPushButton:checked { background: %3; font-weight: 600; }")
                               .arg(Theme::kPanel, Theme::kBorder, Theme::kAccent));
    auto *sideLayout = new QVBoxLayout(sidebar);
    sideLayout->setContentsMargins(16, 24, 16, 16);
    sideLayout->setSpacing(10);

    auto *brand = new QLabel(sidebar);
    brand->setTextFormat(Qt::RichText);
    brand->setText(QStringLiteral(
        "<span style=\"font-size:20px; color:#FFFFFF;\">Gestion de<br>"
        "<span style=\"color:%1; font-weight:700;\">Salles</span></span>").arg(Theme::kAccent));
    brand->setStyleSheet(QStringLiteral("background: transparent;"));
    sideLayout->addWidget(brand);

    auto *line = new QFrame(sidebar);
    line->setFixedHeight(1);
    line->setStyleSheet(QStringLiteral("background: %1; border: none;").arg(Theme::kBorder));
    sideLayout->addWidget(line);
    sideLayout->addSpacing(8);

    const QStringList labels = {QStringLiteral("Accueil"), QStringLiteral("Salles"),
                                QStringLiteral("Enseignants"), QStringLiteral("Matières"),
                                QStringLiteral("Réservations"), QStringLiteral("Planning")};
    for (int i = 0; i < labels.size(); ++i) {
        auto *button = new QPushButton(labels.at(i), sidebar);
        button->setCheckable(true);
        button->setCursor(Qt::PointingHandCursor);
        sideLayout->addWidget(button);
        m_navButtons.append(button);
        connect(button, &QPushButton::clicked, this, [this, i] { showPage(i); });
    }
    sideLayout->addStretch();
    mainLayout->addWidget(sidebar);

    // Content
    auto *content = new QWidget(central);
    auto *contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);

    m_searchBar = new QWidget(content);
    auto *searchLayout = new QHBoxLayout(m_searchBar);
    searchLayout->setContentsMargins(32, 16, 32, 0);
    m_search = new QLineEdit(m_searchBar);
    m_search->setPlaceholderText(QStringLiteral("Recherche…"));
    m_search->setFixedWidth(380);
    searchLayout->addWidget(m_search);
    searchLayout->addStretch();
    contentLayout->addWidget(m_searchBar);

    m_stack = new QStackedWidget(content);
    contentLayout->addWidget(m_stack, 1);
    mainLayout->addWidget(content, 1);

    auto *home = new HomePage;
    auto *rooms = new SallePage;
    auto *teachers = new EnseignantPage;
    auto *subjects = new MatierePage;
    auto *bookings = new ReservationPage;
    auto *planning = new PlanningPage;

    m_stack->addWidget(home);
    m_stack->addWidget(rooms);
    m_stack->addWidget(teachers);
    m_stack->addWidget(subjects);
    m_stack->addWidget(bookings);
    m_stack->addWidget(planning);

    connect(home, &HomePage::pageRequested, this, &MainWindow::showPage);
    for (BasePage *page : {static_cast<BasePage *>(rooms), static_cast<BasePage *>(teachers),
                           static_cast<BasePage *>(subjects), static_cast<BasePage *>(bookings)}) {
        connect(page, &BasePage::homeRequested, this, [this] { showPage(0); });
    }
    connect(planning, &PlanningPage::homeRequested, this, [this] { showPage(0); });

    connect(m_search, &QLineEdit::textChanged, this, [this](const QString &text) {
        if (auto *page = qobject_cast<BasePage *>(m_stack->currentWidget())) {
            page->setSearchText(text);
        }
    });

    showPage(0);
}

void MainWindow::showPage(int index)
{
    m_stack->setCurrentIndex(index);
    for (int i = 0; i < m_navButtons.size(); ++i) {
        m_navButtons[i]->setChecked(i == index);
    }

    m_search->blockSignals(true);
    m_search->clear();
    m_search->blockSignals(false);

    QWidget *current = m_stack->currentWidget();
    if (auto *page = qobject_cast<BasePage *>(current)) {
        page->setSearchText(QString());
        page->reload();
        m_searchBar->show();
    } else {
        if (auto *planning = qobject_cast<PlanningPage *>(current)) {
            planning->reload();
        }
        m_searchBar->hide();
    }
}
