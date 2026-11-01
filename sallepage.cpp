#include "sallepage.h"

#include "database.h"
#include "formdialog.h"
#include "widgets.h"

#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>

namespace {

PageConfig makeConfig()
{
    PageConfig config;
    config.crumb = QStringLiteral("Salles");
    config.titleLead = QStringLiteral("Gestion des ");
    config.titleAccent = QStringLiteral("Salles");
    config.subtitle = QStringLiteral("Consultez les salles disponibles.");
    config.addLabel = QStringLiteral("Ajouter une salle");
    config.statLabels = {QStringLiteral("Total Salles"), QStringLiteral("Salles Disponibles"),
                         QStringLiteral("Salles Occupées")};
    config.headers = {QStringLiteral("Nom de la Salle"), QStringLiteral("Capacité"),
                      QStringLiteral("Équipements"), QStringLiteral("Statut")};
    config.filters = {QStringLiteral("Disponible"), QStringLiteral("Occupée")};
    config.statusColumn = 3;
    return config;
}

}

SallePage::SallePage(QWidget *parent)
    : BasePage(makeConfig(), parent)
{
    reload();
}

QVector<RowData> SallePage::loadRows()
{
    QVector<RowData> result;
    const QVector<QVariantList> rows = Database::rows(QStringLiteral(
        "SELECT id_salle, nom_salle, capacite, COALESCE(equipement, ''), statut "
        "FROM salle ORDER BY nom_salle"));
    for (const QVariantList &row : rows) {
        RowData data;
        data.id = row[0].toInt();
        data.cells = {row[1].toString(), row[2].toString(), row[3].toString(), row[4].toString()};
        result.append(data);
    }
    return result;
}

QVector<int> SallePage::loadStats()
{
    return {Database::scalar(QStringLiteral("SELECT COUNT(*) FROM salle")),
            Database::scalar(QStringLiteral("SELECT COUNT(*) FROM salle WHERE statut = 'Disponible'")),
            Database::scalar(QStringLiteral("SELECT COUNT(*) FROM salle WHERE statut = 'Occupée'"))};
}

void SallePage::addRecord()
{
    showForm(0, false);
}

void SallePage::editRecord(int id)
{
    showForm(id, false);
}

void SallePage::viewRecord(int id)
{
    showForm(id, true);
}

bool SallePage::deleteRecord(int id, QString *error)
{
    if (Database::scalar(QStringLiteral("SELECT COUNT(*) FROM reservation WHERE id_salle = ?"), {id}) > 0) {
        *error = QStringLiteral("Cette salle est liée à des réservations et ne peut pas être supprimée.");
        return false;
    }
    return Database::exec(QStringLiteral("DELETE FROM salle WHERE id_salle = ?"), {id}, error);
}

QString SallePage::deleteMessage() const
{
    return QStringLiteral("Voulez-vous vraiment supprimer cette salle ?");
}

void SallePage::showForm(int id, bool readOnly)
{
    QVariantList current;
    if (id > 0) {
        const QVector<QVariantList> rows = Database::rows(QStringLiteral(
            "SELECT nom_salle, capacite, COALESCE(equipement, ''), statut FROM salle WHERE id_salle = ?"), {id});
        if (rows.isEmpty()) {
            return;
        }
        current = rows.first();
    }

    const QString lead = id == 0 ? QStringLiteral("Ajouter une ")
                                 : (readOnly ? QStringLiteral("Détails de la ") : QStringLiteral("Modifier la "));
    FormDialog dialog(lead, QStringLiteral("Salle"),
                      readOnly ? QStringLiteral("Informations de la salle.")
                               : QStringLiteral("Remplissez les informations ci-dessous."),
                      this);
    QLineEdit *name = dialog.addText(QStringLiteral("Nom de la Salle"), QStringLiteral("Ex : Salle A3"), true);
    QSpinBox *capacity = dialog.addNumber(QStringLiteral("Capacité"), 1, 1000, 30, true);
    QLineEdit *equipment = dialog.addText(QStringLiteral("Équipements"),
                                          QStringLiteral("Ex : Projecteur, Wi-Fi..."));
    QComboBox *status = dialog.addCombo(QStringLiteral("Statut"),
                                        {QStringLiteral("Disponible"), QStringLiteral("Occupée")}, true);

    if (id > 0) {
        name->setText(current[0].toString());
        capacity->setValue(current[1].toInt());
        equipment->setText(current[2].toString());
        status->setCurrentText(current[3].toString());
    }
    dialog.setReadOnly(readOnly);

    dialog.setValidator([&]() -> QString {
        if (name->text().trimmed().isEmpty()) {
            return QStringLiteral("Le nom de la salle est obligatoire.");
        }
        if (Database::scalar(QStringLiteral(
                "SELECT COUNT(*) FROM salle WHERE lower(nom_salle) = lower(?) AND id_salle <> ?"),
                {name->text().trimmed(), id}) > 0) {
            return QStringLiteral("Une salle portant ce nom existe déjà.");
        }
        return QString();
    });

    if (dialog.exec() != QDialog::Accepted || readOnly) {
        return;
    }

    QString error;
    bool ok;
    if (id == 0) {
        ok = Database::exec(QStringLiteral(
            "INSERT INTO salle(nom_salle, capacite, equipement, statut) VALUES(?,?,?,?)"),
            {name->text().trimmed(), capacity->value(), equipment->text().trimmed(),
             status->currentText()}, &error);
    } else {
        ok = Database::exec(QStringLiteral(
            "UPDATE salle SET nom_salle = ?, capacite = ?, equipement = ?, statut = ? WHERE id_salle = ?"),
            {name->text().trimmed(), capacity->value(), equipment->text().trimmed(),
             status->currentText(), id}, &error);
    }
    if (!ok) {
        infoDialog(this, QStringLiteral("Erreur"), error);
    }
}
