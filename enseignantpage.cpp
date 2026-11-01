#include "enseignantpage.h"

#include "database.h"
#include "formdialog.h"
#include "widgets.h"

#include <QComboBox>
#include <QLineEdit>
#include <QRegularExpression>

namespace {

PageConfig makeConfig()
{
    PageConfig config;
    config.crumb = QStringLiteral("Enseignants");
    config.titleLead = QStringLiteral("Gestion des ");
    config.titleAccent = QStringLiteral("Enseignants");
    config.subtitle = QStringLiteral("Gérez les informations des enseignants.");
    config.addLabel = QStringLiteral("Ajouter un enseignant");
    config.statLabels = {QStringLiteral("Total Enseignants"), QStringLiteral("Actifs"),
                         QStringLiteral("Inactifs")};
    config.headers = {QStringLiteral("Nom et prénom"), QStringLiteral("Spécialité"),
                      QStringLiteral("Email"), QStringLiteral("Statut")};
    config.filters = {QStringLiteral("Actif"), QStringLiteral("Inactif")};
    config.statusColumn = 3;
    return config;
}

}

EnseignantPage::EnseignantPage(QWidget *parent)
    : BasePage(makeConfig(), parent)
{
    reload();
}

QVector<RowData> EnseignantPage::loadRows()
{
    QVector<RowData> result;
    const QVector<QVariantList> rows = Database::rows(QStringLiteral(
        "SELECT id_enseignant, nom, prenom, COALESCE(specialite, ''), COALESCE(email, ''), statut "
        "FROM enseignant ORDER BY nom, prenom"));
    for (const QVariantList &row : rows) {
        RowData data;
        data.id = row[0].toInt();
        data.cells = {row[1].toString() + QLatin1Char(' ') + row[2].toString(), row[3].toString(),
                      row[4].toString(), row[5].toString()};
        result.append(data);
    }
    return result;
}

QVector<int> EnseignantPage::loadStats()
{
    return {Database::scalar(QStringLiteral("SELECT COUNT(*) FROM enseignant")),
            Database::scalar(QStringLiteral("SELECT COUNT(*) FROM enseignant WHERE statut = 'Actif'")),
            Database::scalar(QStringLiteral("SELECT COUNT(*) FROM enseignant WHERE statut = 'Inactif'"))};
}

void EnseignantPage::addRecord()
{
    showForm(0, false);
}

void EnseignantPage::editRecord(int id)
{
    showForm(id, false);
}

void EnseignantPage::viewRecord(int id)
{
    showForm(id, true);
}

bool EnseignantPage::deleteRecord(int id, QString *error)
{
    if (Database::scalar(QStringLiteral("SELECT COUNT(*) FROM reservation WHERE id_enseignant = ?"), {id}) > 0) {
        *error = QStringLiteral("Cet enseignant est lié à des réservations et ne peut pas être supprimé.");
        return false;
    }
    return Database::exec(QStringLiteral("DELETE FROM enseignant WHERE id_enseignant = ?"), {id}, error);
}

QString EnseignantPage::deleteMessage() const
{
    return QStringLiteral("Voulez-vous vraiment supprimer cet enseignant ?");
}

void EnseignantPage::showForm(int id, bool readOnly)
{
    QVariantList current;
    if (id > 0) {
        const QVector<QVariantList> rows = Database::rows(QStringLiteral(
            "SELECT nom, prenom, COALESCE(email, ''), COALESCE(specialite, ''), statut "
            "FROM enseignant WHERE id_enseignant = ?"), {id});
        if (rows.isEmpty()) {
            return;
        }
        current = rows.first();
    }

    const QString lead = id == 0 ? QStringLiteral("Ajouter un ")
                                 : (readOnly ? QStringLiteral("Détails de l'") : QStringLiteral("Modifier l'"));
    FormDialog dialog(lead, QStringLiteral("Enseignant"),
                      readOnly ? QStringLiteral("Informations de l'enseignant.")
                               : QStringLiteral("Remplissez les informations ci-dessous."),
                      this);
    QLineEdit *lastName = dialog.addText(QStringLiteral("Nom"), QStringLiteral("Ex : Martin"), true);
    QLineEdit *firstName = dialog.addText(QStringLiteral("Prénom"), QStringLiteral("Ex : Paul"), true);
    QLineEdit *email = dialog.addText(QStringLiteral("Email"), QStringLiteral("Ex : paul.martin@example.com"), true);
    QLineEdit *field = dialog.addText(QStringLiteral("Spécialité"), QStringLiteral("Ex : Mathématique"));
    QComboBox *status = dialog.addCombo(QStringLiteral("Statut"),
                                        {QStringLiteral("Actif"), QStringLiteral("Inactif")}, true);

    if (id > 0) {
        lastName->setText(current[0].toString());
        firstName->setText(current[1].toString());
        email->setText(current[2].toString());
        field->setText(current[3].toString());
        status->setCurrentText(current[4].toString());
    }
    dialog.setReadOnly(readOnly);

    dialog.setValidator([&]() -> QString {
        if (lastName->text().trimmed().isEmpty() || firstName->text().trimmed().isEmpty()) {
            return QStringLiteral("Le nom et le prénom sont obligatoires.");
        }
        static const QRegularExpression pattern(QStringLiteral("^[^@\\s]+@[^@\\s]+\\.[^@\\s]+$"));
        if (!pattern.match(email->text().trimmed()).hasMatch()) {
            return QStringLiteral("L'adresse email n'est pas valide.");
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
            "INSERT INTO enseignant(nom, prenom, email, specialite, statut) VALUES(?,?,?,?,?)"),
            {lastName->text().trimmed(), firstName->text().trimmed(), email->text().trimmed(),
             field->text().trimmed(), status->currentText()}, &error);
    } else {
        ok = Database::exec(QStringLiteral(
            "UPDATE enseignant SET nom = ?, prenom = ?, email = ?, specialite = ?, statut = ? "
            "WHERE id_enseignant = ?"),
            {lastName->text().trimmed(), firstName->text().trimmed(), email->text().trimmed(),
             field->text().trimmed(), status->currentText(), id}, &error);
    }
    if (!ok) {
        infoDialog(this, QStringLiteral("Erreur"), error);
    }
}
