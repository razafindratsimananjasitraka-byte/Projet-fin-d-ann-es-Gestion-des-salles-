#include "reservationpage.h"

#include "database.h"
#include "formdialog.h"
#include "theme.h"
#include "widgets.h"

#include <QComboBox>
#include <QDate>
#include <QDateEdit>
#include <QTime>
#include <QTimeEdit>

namespace {

PageConfig makeConfig()
{
    PageConfig config;
    config.crumb = QStringLiteral("Réservations");
    config.titleLead = QStringLiteral("Gestion des ");
    config.titleAccent = QStringLiteral("Réservations");
    config.subtitle = QStringLiteral("Gérez les réservations des salles.");
    config.addLabel = QStringLiteral("Ajouter une réservation");
    config.statLabels = {QStringLiteral("Total Réservations"), QStringLiteral("Réservations Actives"),
                         QStringLiteral("Réservations Annulées")};
    config.headers = {QStringLiteral("Salle"), QStringLiteral("Date"), QStringLiteral("Horaire"),
                      QStringLiteral("Enseignant"), QStringLiteral("Matière"), QStringLiteral("Statut")};
    config.filters = {QStringLiteral("Active"), QStringLiteral("Annulée")};
    config.statusColumn = 5;
    return config;
}

void selectData(QComboBox *combo, const QVariant &value)
{
    const int index = combo->findData(value);
    if (index >= 0) {
        combo->setCurrentIndex(index);
    }
}

}

ReservationPage::ReservationPage(QWidget *parent)
    : BasePage(makeConfig(), parent)
{
    reload();
}

QVector<RowData> ReservationPage::loadRows()
{
    QVector<RowData> result;
    const QVector<QVariantList> rows = Database::rows(QStringLiteral(
        "SELECT r.id_reservation, s.nom_salle, r.date_reservation, r.heure_debut, r.heure_fin, "
        "e.nom, e.prenom, m.intitule, r.statut "
        "FROM reservation r "
        "JOIN salle s ON s.id_salle = r.id_salle "
        "JOIN enseignant e ON e.id_enseignant = r.id_enseignant "
        "JOIN matiere m ON m.id_matiere = r.id_matiere "
        "ORDER BY r.date_reservation DESC, r.heure_debut"));
    for (const QVariantList &row : rows) {
        RowData data;
        data.id = row[0].toInt();
        data.cells = {row[1].toString(),
                      Database::displayDate(row[2].toString()),
                      row[3].toString() + QStringLiteral(" - ") + row[4].toString(),
                      row[5].toString() + QLatin1Char(' ') + row[6].toString(),
                      row[7].toString(),
                      row[8].toString()};
        result.append(data);
    }
    return result;
}

QVector<int> ReservationPage::loadStats()
{
    return {Database::scalar(QStringLiteral("SELECT COUNT(*) FROM reservation")),
            Database::scalar(QStringLiteral("SELECT COUNT(*) FROM reservation WHERE statut = 'Active'")),
            Database::scalar(QStringLiteral("SELECT COUNT(*) FROM reservation WHERE statut = 'Annulée'"))};
}

void ReservationPage::addRecord()
{
    showForm(0, false);
}

void ReservationPage::editRecord(int id)
{
    showForm(id, false);
}

void ReservationPage::viewRecord(int id)
{
    showForm(id, true);
}

bool ReservationPage::deleteRecord(int id, QString *error)
{
    return Database::exec(QStringLiteral("DELETE FROM reservation WHERE id_reservation = ?"), {id}, error);
}

QString ReservationPage::deleteMessage() const
{
    return QStringLiteral("Voulez-vous vraiment supprimer cette réservation ?");
}

void ReservationPage::showForm(int id, bool readOnly)
{
    QVariantList current;
    if (id > 0) {
        const QVector<QVariantList> rows = Database::rows(QStringLiteral(
            "SELECT id_salle, id_enseignant, id_matiere, date_reservation, heure_debut, heure_fin, statut "
            "FROM reservation WHERE id_reservation = ?"), {id});
        if (rows.isEmpty()) {
            return;
        }
        current = rows.first();
    }
    const int currentRoom = id > 0 ? current[0].toInt() : 0;
    const int currentTeacher = id > 0 ? current[1].toInt() : 0;
    const int currentSubject = id > 0 ? current[2].toInt() : 0;

    const QString lead = id == 0 ? QStringLiteral("Ajouter une ")
                                 : (readOnly ? QStringLiteral("Détails de la ") : QStringLiteral("Modifier la "));
    FormDialog dialog(lead, QStringLiteral("Réservation"),
                      readOnly ? QStringLiteral("Informations de la réservation.")
                               : QStringLiteral("Remplissez les informations ci-dessous."),
                      this);

    QComboBox *room = dialog.addCombo(QStringLiteral("Salle"), QStringList(), true);
    QComboBox *teacher = dialog.addCombo(QStringLiteral("Enseignant"), QStringList(), true);
    QComboBox *subject = dialog.addCombo(QStringLiteral("Matière"), QStringList(), true);
    QDateEdit *date = dialog.addDate(QStringLiteral("Date"), QDate::currentDate(), true);
    QTimeEdit *start = dialog.addTime(QStringLiteral("Heure de début"), QTime(8, 0), true);
    QTimeEdit *end = dialog.addTime(QStringLiteral("Heure de fin"), QTime(10, 0), true);
    QComboBox *status = dialog.addCombo(QStringLiteral("Statut"),
                                        {QStringLiteral("Active"), QStringLiteral("Annulée")}, true);

    // Only available rooms, active teachers and active subjects can be booked.
    for (const QVariantList &row : Database::rows(QStringLiteral(
             "SELECT id_salle, nom_salle FROM salle WHERE statut = 'Disponible' OR id_salle = ? ORDER BY nom_salle"),
             {currentRoom})) {
        room->addItem(row[1].toString(), row[0]);
    }
    for (const QVariantList &row : Database::rows(QStringLiteral(
             "SELECT id_enseignant, nom || ' ' || prenom FROM enseignant "
             "WHERE statut = 'Actif' OR id_enseignant = ? ORDER BY nom, prenom"),
             {currentTeacher})) {
        teacher->addItem(row[1].toString(), row[0]);
    }
    for (const QVariantList &row : Database::rows(QStringLiteral(
             "SELECT id_matiere, intitule FROM matiere WHERE statut = 'Active' OR id_matiere = ? ORDER BY intitule"),
             {currentSubject})) {
        subject->addItem(row[1].toString(), row[0]);
    }

    if (id > 0) {
        selectData(room, currentRoom);
        selectData(teacher, currentTeacher);
        selectData(subject, currentSubject);
        date->setDate(QDate::fromString(current[3].toString(), Qt::ISODate));
        start->setTime(QTime::fromString(current[4].toString(), QStringLiteral("HH:mm")));
        end->setTime(QTime::fromString(current[5].toString(), QStringLiteral("HH:mm")));
        status->setCurrentText(current[6].toString());
    }
    dialog.setReadOnly(readOnly);

    dialog.setValidator([&]() -> QString {
        if (room->count() == 0) {
            return QStringLiteral("Aucune salle disponible. Ajoutez ou libérez une salle.");
        }
        if (teacher->count() == 0) {
            return QStringLiteral("Aucun enseignant actif. Ajoutez un enseignant.");
        }
        if (subject->count() == 0) {
            return QStringLiteral("Aucune matière active. Ajoutez une matière.");
        }
        if (end->time() <= start->time()) {
            return QStringLiteral("L'heure de fin doit être postérieure à l'heure de début.");
        }
        if (start->time().hour() < Schedule::kFirstHour || end->time() > QTime(Schedule::kLastHour, 0)) {
            return QStringLiteral("Les créneaux doivent être compris entre %1:00 et %2:00.")
                .arg(Schedule::kFirstHour, 2, 10, QLatin1Char('0'))
                .arg(Schedule::kLastHour);
        }
        if (status->currentText() == QLatin1String("Active")) {
            return Database::findConflict(room->currentData().toInt(), teacher->currentData().toInt(),
                                          date->date().toString(Qt::ISODate),
                                          start->time().toString(QStringLiteral("HH:mm")),
                                          end->time().toString(QStringLiteral("HH:mm")), id);
        }
        return QString();
    });

    if (dialog.exec() != QDialog::Accepted || readOnly) {
        return;
    }

    const QVariantList values = {room->currentData().toInt(), teacher->currentData().toInt(),
                                 subject->currentData().toInt(), date->date().toString(Qt::ISODate),
                                 start->time().toString(QStringLiteral("HH:mm")),
                                 end->time().toString(QStringLiteral("HH:mm")), status->currentText()};
    QString error;
    bool ok;
    if (id == 0) {
        ok = Database::exec(QStringLiteral(
            "INSERT INTO reservation(id_salle, id_enseignant, id_matiere, date_reservation, "
            "heure_debut, heure_fin, statut) VALUES(?,?,?,?,?,?,?)"), values, &error);
    } else {
        QVariantList updateValues = values;
        updateValues << id;
        ok = Database::exec(QStringLiteral(
            "UPDATE reservation SET id_salle = ?, id_enseignant = ?, id_matiere = ?, date_reservation = ?, "
            "heure_debut = ?, heure_fin = ?, statut = ? WHERE id_reservation = ?"), updateValues, &error);
    }
    if (!ok) {
        infoDialog(this, QStringLiteral("Erreur"), error);
    }
}
