#include "matierepage.h"

#include "database.h"
#include "formdialog.h"
#include "widgets.h"

#include <QComboBox>
#include <QLineEdit>

namespace {

PageConfig makeConfig()
{
    PageConfig config;
    config.crumb = QStringLiteral("Matières");
    config.titleLead = QStringLiteral("Gestion des ");
    config.titleAccent = QStringLiteral("Matières");
    config.subtitle = QStringLiteral("Organisez les matières enseignées.");
    config.addLabel = QStringLiteral("Ajouter une matière");
    config.statLabels = {QStringLiteral("Total Matières"), QStringLiteral("Matières Actives"),
                         QStringLiteral("Matières Inactives")};
    config.headers = {QStringLiteral("Nom de la Matière"), QStringLiteral("Code"), QStringLiteral("Statut")};
    config.filters = {QStringLiteral("Active"), QStringLiteral("Inactive")};
    config.statusColumn = 2;
    return config;
}

}

MatierePage::MatierePage(QWidget *parent)
    : BasePage(makeConfig(), parent)
{
    reload();
}

QVector<RowData> MatierePage::loadRows()
{
    QVector<RowData> result;
    const QVector<QVariantList> rows = Database::rows(QStringLiteral(
        "SELECT id_matiere, intitule, code, statut FROM matiere ORDER BY intitule"));
    for (const QVariantList &row : rows) {
        RowData data;
        data.id = row[0].toInt();
        data.cells = {row[1].toString(), row[2].toString(), row[3].toString()};
        result.append(data);
    }
    return result;
}

QVector<int> MatierePage::loadStats()
{
    return {Database::scalar(QStringLiteral("SELECT COUNT(*) FROM matiere")),
            Database::scalar(QStringLiteral("SELECT COUNT(*) FROM matiere WHERE statut = 'Active'")),
            Database::scalar(QStringLiteral("SELECT COUNT(*) FROM matiere WHERE statut = 'Inactive'"))};
}

void MatierePage::addRecord()
{
    showForm(0, false);
}

void MatierePage::editRecord(int id)
{
    showForm(id, false);
}

void MatierePage::viewRecord(int id)
{
    showForm(id, true);
}

bool MatierePage::deleteRecord(int id, QString *error)
{
    if (Database::scalar(QStringLiteral("SELECT COUNT(*) FROM reservation WHERE id_matiere = ?"), {id}) > 0) {
        *error = QStringLiteral("Cette matière est liée à des réservations et ne peut pas être supprimée.");
        return false;
    }
    return Database::exec(QStringLiteral("DELETE FROM matiere WHERE id_matiere = ?"), {id}, error);
}

QString MatierePage::deleteMessage() const
{
    return QStringLiteral("Voulez-vous vraiment supprimer cette matière ?");
}

void MatierePage::showForm(int id, bool readOnly)
{
    QVariantList current;
    if (id > 0) {
        const QVector<QVariantList> rows = Database::rows(QStringLiteral(
            "SELECT code, intitule, statut FROM matiere WHERE id_matiere = ?"), {id});
        if (rows.isEmpty()) {
            return;
        }
        current = rows.first();
    }

    const QString lead = id == 0 ? QStringLiteral("Ajouter une ")
                                 : (readOnly ? QStringLiteral("Détails de la ") : QStringLiteral("Modifier la "));
    FormDialog dialog(lead, QStringLiteral("Matière"),
                      readOnly ? QStringLiteral("Informations de la matière.")
                               : QStringLiteral("Remplissez les informations ci-dessous."),
                      this);
    QLineEdit *code = dialog.addText(QStringLiteral("Code"), QStringLiteral("Ex : MAT"), true);
    QLineEdit *title = dialog.addText(QStringLiteral("Intitulé"), QStringLiteral("Ex : Mathématique"), true);
    QComboBox *status = dialog.addCombo(QStringLiteral("Statut"),
                                        {QStringLiteral("Active"), QStringLiteral("Inactive")}, true);

    if (id > 0) {
        code->setText(current[0].toString());
        title->setText(current[1].toString());
        status->setCurrentText(current[2].toString());
    }
    dialog.setReadOnly(readOnly);

    dialog.setValidator([&]() -> QString {
        if (code->text().trimmed().isEmpty() || title->text().trimmed().isEmpty()) {
            return QStringLiteral("Le code et l'intitulé sont obligatoires.");
        }
        if (Database::scalar(QStringLiteral(
                "SELECT COUNT(*) FROM matiere WHERE lower(code) = lower(?) AND id_matiere <> ?"),
                {code->text().trimmed(), id}) > 0) {
            return QStringLiteral("Une matière avec ce code existe déjà.");
        }
        return QString();
    });

    if (dialog.exec() != QDialog::Accepted || readOnly) {
        return;
    }

    QString error;
    bool ok;
    if (id == 0) {
        ok = Database::exec(QStringLiteral("INSERT INTO matiere(code, intitule, statut) VALUES(?,?,?)"),
                            {code->text().trimmed().toUpper(), title->text().trimmed(),
                             status->currentText()}, &error);
    } else {
        ok = Database::exec(QStringLiteral(
            "UPDATE matiere SET code = ?, intitule = ?, statut = ? WHERE id_matiere = ?"),
            {code->text().trimmed().toUpper(), title->text().trimmed(), status->currentText(), id}, &error);
    }
    if (!ok) {
        infoDialog(this, QStringLiteral("Erreur"), error);
    }
}
