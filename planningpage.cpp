#include "planningpage.h"

#include "database.h"
#include "theme.h"
#include "widgets.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QSet>
#include <QTableWidget>
#include <QVBoxLayout>

namespace {

const QStringList &dayNames()
{
    static const QStringList names = {
        QStringLiteral("Lundi"), QStringLiteral("Mardi"), QStringLiteral("Mercredi"),
        QStringLiteral("Jeudi"), QStringLiteral("Vendredi"), QStringLiteral("Samedi"),
        QStringLiteral("Dimanche")};
    return names;
}

int toMinutes(const QString &time)
{
    return time.left(2).toInt() * 60 + time.mid(3, 2).toInt();
}

constexpr int kSlotCount = Schedule::kLastHour - Schedule::kFirstHour;

}

PlanningPage::PlanningPage(QWidget *parent)
    : QWidget(parent)
{
    const QDate today = QDate::currentDate();
    m_weekStart = today.addDays(1 - today.dayOfWeek());

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(32, 12, 32, 20);
    root->setSpacing(14);

    auto *header = new PageHeader(QStringLiteral("Planning"), QStringLiteral("Planning "),
                                  QStringLiteral("hebdomadaire"),
                                  QStringLiteral("Visualisez les réservations de la semaine et les conflits horaires."),
                                  this);
    connect(header, &PageHeader::homeRequested, this, &PlanningPage::homeRequested);
    root->addWidget(header);

    auto *statsRow = new QHBoxLayout;
    statsRow->setSpacing(24);
    for (int i = 0; i < 3; ++i) {
        m_stats[i] = new StatCard(this);
        statsRow->addWidget(m_stats[i]);
    }
    root->addLayout(statsRow);

    auto *toolbar = new QHBoxLayout;
    toolbar->setSpacing(12);
    QPushButton *previous = makeButton(QStringLiteral("Semaine précédente"), Theme::kPanel,
                                       QStringLiteral("#FFFFFF"), this);
    QPushButton *today_ = makeButton(QStringLiteral("Aujourd'hui"), Theme::kAccent,
                                     QStringLiteral("#000000"), this);
    QPushButton *next = makeButton(QStringLiteral("Semaine suivante"), Theme::kPanel,
                                   QStringLiteral("#FFFFFF"), this);
    for (QPushButton *button : {previous, today_, next}) {
        button->setMinimumHeight(40);
    }
    m_weekLabel = new QLabel(this);
    m_weekLabel->setStyleSheet(QStringLiteral("font-size: 15px; font-weight: 600; color: #FFFFFF;"));
    m_roomFilter = new QComboBox(this);
    m_roomFilter->setMinimumWidth(200);

    toolbar->addWidget(previous);
    toolbar->addWidget(today_);
    toolbar->addWidget(next);
    toolbar->addSpacing(8);
    toolbar->addWidget(m_weekLabel);
    toolbar->addStretch();
    toolbar->addWidget(m_roomFilter);
    root->addLayout(toolbar);

    m_grid = new QTableWidget(kSlotCount, 7, this);
    m_grid->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_grid->setSelectionMode(QAbstractItemView::NoSelection);
    m_grid->setFocusPolicy(Qt::NoFocus);
    m_grid->setWordWrap(true);
    m_grid->setTextElideMode(Qt::ElideNone);
    m_grid->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_grid->horizontalHeader()->setHighlightSections(false);
    m_grid->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    m_grid->verticalHeader()->setDefaultSectionSize(72);
    m_grid->verticalHeader()->setHighlightSections(false);
    QStringList hours;
    for (int h = Schedule::kFirstHour; h < Schedule::kLastHour; ++h) {
        hours << QStringLiteral("%1:00").arg(h, 2, 10, QLatin1Char('0'));
    }
    m_grid->setVerticalHeaderLabels(hours);
    m_grid->setStyleSheet(QStringLiteral(
        "QTableWidget { background: %1; border: 1px solid %2; border-radius: 12px; gridline-color: #2A3650;"
        " outline: 0; color: #FFFFFF; }"
        "QTableWidget::item { padding: 4px; }"
        "QHeaderView::section { background: %3; color: #FFFFFF; padding: 8px; border: none;"
        " border-right: 1px solid #2A3650; border-bottom: 1px solid #2A3650; font-weight: 600; }"
        "QTableCornerButton::section { background: %3; border: none; }")
                              .arg(Theme::kBackground, Theme::kBorder, Theme::kPanel));
    root->addWidget(m_grid, 1);

    connect(previous, &QPushButton::clicked, this, [this] { shiftWeek(-7); });
    connect(next, &QPushButton::clicked, this, [this] { shiftWeek(7); });
    connect(today_, &QPushButton::clicked, this, [this] {
        const QDate now = QDate::currentDate();
        m_weekStart = now.addDays(1 - now.dayOfWeek());
        fill();
    });
    connect(m_roomFilter, &QComboBox::currentTextChanged, this, [this] { fill(); });

    reload();
}

void PlanningPage::shiftWeek(int days)
{
    m_weekStart = m_weekStart.addDays(days);
    fill();
}

void PlanningPage::reload()
{
    const int selected = m_roomFilter->currentData().toInt();
    m_roomFilter->blockSignals(true);
    m_roomFilter->clear();
    m_roomFilter->addItem(QStringLiteral("Toutes les salles"), 0);
    const QVector<QVariantList> rooms = Database::rows(
        QStringLiteral("SELECT id_salle, nom_salle FROM salle ORDER BY nom_salle"));
    for (const QVariantList &room : rooms) {
        m_roomFilter->addItem(room[1].toString(), room[0]);
    }
    const int index = m_roomFilter->findData(selected);
    m_roomFilter->setCurrentIndex(index >= 0 ? index : 0);
    m_roomFilter->blockSignals(false);
    fill();
}

void PlanningPage::fill()
{
    const QDate weekEnd = m_weekStart.addDays(6);
    m_weekLabel->setText(QStringLiteral("Semaine du %1 au %2")
                             .arg(m_weekStart.toString(QStringLiteral("dd/MM/yyyy")),
                                  weekEnd.toString(QStringLiteral("dd/MM/yyyy"))));

    QStringList headers;
    for (int i = 0; i < 7; ++i) {
        headers << QStringLiteral("%1\n%2").arg(dayNames().at(i),
                                                 m_weekStart.addDays(i).toString(QStringLiteral("dd/MM")));
    }
    m_grid->setHorizontalHeaderLabels(headers);
    m_grid->clearContents();

    const QSet<int> conflicts = Database::conflictingIds();
    const int roomId = m_roomFilter->currentData().toInt();

    QString sql = QStringLiteral(
        "SELECT r.id_reservation, r.id_salle, r.date_reservation, r.heure_debut, r.heure_fin, "
        "s.nom_salle, e.nom || ' ' || e.prenom, m.code "
        "FROM reservation r "
        "JOIN salle s ON s.id_salle = r.id_salle "
        "JOIN enseignant e ON e.id_enseignant = r.id_enseignant "
        "JOIN matiere m ON m.id_matiere = r.id_matiere "
        "WHERE r.statut = 'Active' AND r.date_reservation BETWEEN ? AND ?");
    QVariantList args = {m_weekStart.toString(Qt::ISODate), weekEnd.toString(Qt::ISODate)};
    if (roomId > 0) {
        sql += QStringLiteral(" AND r.id_salle = ?");
        args << roomId;
    }
    sql += QStringLiteral(" ORDER BY r.heure_debut");

    const QVector<QVariantList> rows = Database::rows(sql, args);
    QSet<int> usedRooms;
    int conflictCount = 0;

    for (const QVariantList &row : rows) {
        const int reservationId = row[0].toInt();
        const QDate date = QDate::fromString(row[2].toString(), Qt::ISODate);
        const int day = m_weekStart.daysTo(date);
        if (day < 0 || day > 6) {
            continue;
        }
        const int startMinutes = toMinutes(row[3].toString());
        const int endMinutes = toMinutes(row[4].toString());
        const bool inConflict = conflicts.contains(reservationId);
        usedRooms.insert(row[1].toInt());
        if (inConflict) {
            ++conflictCount;
        }

        const QString text = QStringLiteral("%1 - %2\n%3 · %4\n%5")
                                 .arg(row[3].toString(), row[4].toString(), row[5].toString(),
                                      row[7].toString(), row[6].toString());

        for (int hour = Schedule::kFirstHour; hour < Schedule::kLastHour; ++hour) {
            const int slotStart = hour * 60;
            const int slotEnd = slotStart + 60;
            if (!(startMinutes < slotEnd && endMinutes > slotStart)) {
                continue;
            }
            const int tableRow = hour - Schedule::kFirstHour;
            QTableWidgetItem *item = m_grid->item(tableRow, day);
            if (!item) {
                item = new QTableWidgetItem;
                item->setFlags(Qt::ItemIsEnabled);
                item->setForeground(QColor(QStringLiteral("#FFFFFF")));
                m_grid->setItem(tableRow, day, item);
            }
            if (inConflict) {
                item->setData(Qt::UserRole, true);
            }
            item->setBackground(QColor(item->data(Qt::UserRole).toBool() ? QStringLiteral("#7A1F1F")
                                                                         : QStringLiteral("#6B5A00")));
            if (startMinutes >= slotStart) {
                item->setText(item->text().isEmpty() ? text : item->text() + QStringLiteral("\n\n") + text);
            }
        }
    }

    m_stats[0]->setContent(QStringLiteral("Réservations de la semaine"), rows.size());
    m_stats[1]->setContent(QStringLiteral("Salles utilisées"), usedRooms.size());
    m_stats[2]->setContent(QStringLiteral("Conflits détectés"), conflictCount);
}
