#include "database.h"

#include <QDate>
#include <QDebug>
#include <QDir>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QStandardPaths>

bool Database::open(QString *error)
{
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dir);

    QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"));
    db.setDatabaseName(dir + QStringLiteral("/reservations.db"));
    if (!db.open()) {
        if (error) {
            *error = db.lastError().text();
        }
        return false;
    }

    if (!exec(QStringLiteral("PRAGMA foreign_keys = ON"), QVariantList(), error)) {
        return false;
    }
    return createSchema(error) && seed(error);
}

bool Database::exec(const QString &sql, const QVariantList &args, QString *error)
{
    QSqlQuery query(QSqlDatabase::database());
    if (!query.prepare(sql)) {
        if (error) {
            *error = query.lastError().text();
        }
        return false;
    }
    for (const QVariant &value : args) {
        query.addBindValue(value);
    }
    if (!query.exec()) {
        if (error) {
            *error = query.lastError().text();
        }
        qWarning() << query.lastError().text() << sql;
        return false;
    }
    return true;
}

QVector<QVariantList> Database::rows(const QString &sql, const QVariantList &args)
{
    QVector<QVariantList> result;
    QSqlQuery query(QSqlDatabase::database());
    if (!query.prepare(sql)) {
        qWarning() << query.lastError().text() << sql;
        return result;
    }
    for (const QVariant &value : args) {
        query.addBindValue(value);
    }
    if (!query.exec()) {
        qWarning() << query.lastError().text() << sql;
        return result;
    }
    const int columns = query.record().count();
    while (query.next()) {
        QVariantList row;
        for (int i = 0; i < columns; ++i) {
            row << query.value(i);
        }
        result << row;
    }
    return result;
}

int Database::scalar(const QString &sql, const QVariantList &args)
{
    const QVector<QVariantList> result = rows(sql, args);
    if (result.isEmpty() || result.first().isEmpty()) {
        return 0;
    }
    return result.first().first().toInt();
}

QString Database::findConflict(int roomId, int teacherId, const QString &date,
                               const QString &start, const QString &end, int excludeId)
{
    // Two slots overlap when each one starts before the other one ends.
    QVector<QVariantList> found = rows(
        QStringLiteral("SELECT heure_debut, heure_fin FROM reservation "
                       "WHERE id_salle = ? AND date_reservation = ? AND statut = 'Active' "
                       "AND id_reservation <> ? AND heure_debut < ? AND ? < heure_fin LIMIT 1"),
        {roomId, date, excludeId, end, start});
    if (!found.isEmpty()) {
        return QStringLiteral("Cette salle est déjà réservée de %1 à %2 sur ce créneau.")
            .arg(found[0][0].toString(), found[0][1].toString());
    }

    found = rows(
        QStringLiteral("SELECT r.heure_debut, r.heure_fin, s.nom_salle FROM reservation r "
                       "JOIN salle s ON s.id_salle = r.id_salle "
                       "WHERE r.id_enseignant = ? AND r.date_reservation = ? AND r.statut = 'Active' "
                       "AND r.id_reservation <> ? AND r.heure_debut < ? AND ? < r.heure_fin LIMIT 1"),
        {teacherId, date, excludeId, end, start});
    if (!found.isEmpty()) {
        return QStringLiteral("Cet enseignant est déjà affecté à la salle %3 de %1 à %2.")
            .arg(found[0][0].toString(), found[0][1].toString(), found[0][2].toString());
    }
    return QString();
}

QSet<int> Database::conflictingIds()
{
    QSet<int> ids;
    const QVector<QVariantList> pairs = rows(QStringLiteral(
        "SELECT a.id_reservation, b.id_reservation FROM reservation a "
        "JOIN reservation b ON a.id_reservation < b.id_reservation "
        "AND a.date_reservation = b.date_reservation "
        "AND a.statut = 'Active' AND b.statut = 'Active' "
        "AND a.heure_debut < b.heure_fin AND b.heure_debut < a.heure_fin "
        "AND (a.id_salle = b.id_salle OR a.id_enseignant = b.id_enseignant)"));
    for (const QVariantList &pair : pairs) {
        ids.insert(pair[0].toInt());
        ids.insert(pair[1].toInt());
    }
    return ids;
}

QString Database::displayDate(const QString &isoDate)
{
    return QDate::fromString(isoDate, Qt::ISODate).toString(QStringLiteral("dd/MM/yyyy"));
}

bool Database::createSchema(QString *error)
{
    const QStringList statements = {
        QStringLiteral("CREATE TABLE IF NOT EXISTS salle ("
                       "id_salle INTEGER PRIMARY KEY AUTOINCREMENT, "
                       "nom_salle TEXT NOT NULL UNIQUE, "
                       "capacite INTEGER NOT NULL CHECK (capacite > 0), "
                       "equipement TEXT, "
                       "statut TEXT NOT NULL DEFAULT 'Disponible')"),
        QStringLiteral("CREATE TABLE IF NOT EXISTS enseignant ("
                       "id_enseignant INTEGER PRIMARY KEY AUTOINCREMENT, "
                       "nom TEXT NOT NULL, "
                       "prenom TEXT NOT NULL, "
                       "email TEXT, "
                       "specialite TEXT, "
                       "statut TEXT NOT NULL DEFAULT 'Actif')"),
        QStringLiteral("CREATE TABLE IF NOT EXISTS matiere ("
                       "id_matiere INTEGER PRIMARY KEY AUTOINCREMENT, "
                       "code TEXT NOT NULL UNIQUE, "
                       "intitule TEXT NOT NULL, "
                       "statut TEXT NOT NULL DEFAULT 'Active')"),
        QStringLiteral("CREATE TABLE IF NOT EXISTS reservation ("
                       "id_reservation INTEGER PRIMARY KEY AUTOINCREMENT, "
                       "id_salle INTEGER NOT NULL REFERENCES salle(id_salle) ON DELETE RESTRICT, "
                       "id_enseignant INTEGER NOT NULL REFERENCES enseignant(id_enseignant) ON DELETE RESTRICT, "
                       "id_matiere INTEGER NOT NULL REFERENCES matiere(id_matiere) ON DELETE RESTRICT, "
                       "date_reservation TEXT NOT NULL, "
                       "heure_debut TEXT NOT NULL, "
                       "heure_fin TEXT NOT NULL, "
                       "statut TEXT NOT NULL DEFAULT 'Active', "
                       "CHECK (heure_fin > heure_debut))"),
        QStringLiteral("CREATE INDEX IF NOT EXISTS idx_reservation_date "
                       "ON reservation(date_reservation)")};

    for (const QString &statement : statements) {
        if (!exec(statement, QVariantList(), error)) {
            return false;
        }
    }
    return true;
}

bool Database::seed(QString *error)
{
    if (scalar(QStringLiteral("SELECT COUNT(*) FROM salle")) > 0) {
        return true;
    }

    QSqlDatabase::database().transaction();

    struct Room { const char *name; int capacity; const char *equipment; const char *status; };
    const Room roomList[] = {
        {"Salle A1", 30, "Vidéo projecteur", "Disponible"},
        {"Salle A2", 30, "Matériel labo", "Occupée"},
        {"Salle B1", 40, "Ordinateurs", "Disponible"},
        {"Salle B2", 25, "Tableau interactif", "Disponible"},
        {"Salle C1", 20, "Projecteur, Wi-Fi", "Disponible"},
        {"Salle C2", 35, "Ordinateurs, Wi-Fi", "Occupée"}};
    for (const Room &room : roomList) {
        exec(QStringLiteral("INSERT INTO salle(nom_salle, capacite, equipement, statut) VALUES(?,?,?,?)"),
             {QString::fromUtf8(room.name), room.capacity, QString::fromUtf8(room.equipment),
              QString::fromUtf8(room.status)}, error);
    }

    struct Teacher { const char *last; const char *first; const char *mail; const char *field; const char *status; };
    const Teacher teacherList[] = {
        {"Martin", "Paul", "paul.martin@example.com", "Mathématique", "Actif"},
        {"Bernard", "Claire", "claire.bernard@example.com", "Anglais", "Actif"},
        {"Dubois", "Luc", "luc.dubois@example.com", "Informatique", "Actif"},
        {"Petit", "Anne", "anne.petit@example.com", "Physique", "Actif"},
        {"Moreau", "Jean", "jean.moreau@example.com", "Français", "Inactif"}};
    for (const Teacher &teacher : teacherList) {
        exec(QStringLiteral("INSERT INTO enseignant(nom, prenom, email, specialite, statut) VALUES(?,?,?,?,?)"),
             {QString::fromUtf8(teacher.last), QString::fromUtf8(teacher.first),
              QString::fromUtf8(teacher.mail), QString::fromUtf8(teacher.field),
              QString::fromUtf8(teacher.status)}, error);
    }

    struct Subject { const char *code; const char *title; const char *status; };
    const Subject subjectList[] = {
        {"MAT", "Mathématique", "Active"},
        {"ANG", "Anglais", "Active"},
        {"INF", "Informatique", "Active"},
        {"PHY", "Physique", "Active"},
        {"FRA", "Français", "Inactive"}};
    for (const Subject &subject : subjectList) {
        exec(QStringLiteral("INSERT INTO matiere(code, intitule, statut) VALUES(?,?,?)"),
             {QString::fromUtf8(subject.code), QString::fromUtf8(subject.title),
              QString::fromUtf8(subject.status)}, error);
    }

    const QDate today = QDate::currentDate();
    const QDate monday = today.addDays(1 - today.dayOfWeek());
    struct Booking { int room; int teacher; int subject; int day; const char *start; const char *end; const char *status; };
    const Booking bookingList[] = {
        {1, 1, 1, 0, "08:00", "10:00", "Active"},
        {3, 2, 2, 0, "10:00", "12:00", "Active"},
        {1, 3, 3, 1, "14:00", "16:00", "Active"},
        {5, 1, 4, 2, "08:00", "11:00", "Active"},
        {3, 4, 1, 3, "09:00", "12:00", "Active"},
        {1, 2, 2, 4, "13:00", "15:00", "Annulée"}};
    for (const Booking &booking : bookingList) {
        exec(QStringLiteral("INSERT INTO reservation(id_salle, id_enseignant, id_matiere, "
                            "date_reservation, heure_debut, heure_fin, statut) VALUES(?,?,?,?,?,?,?)"),
             {booking.room, booking.teacher, booking.subject,
              monday.addDays(booking.day).toString(Qt::ISODate),
              QString::fromUtf8(booking.start), QString::fromUtf8(booking.end),
              QString::fromUtf8(booking.status)}, error);
    }

    return QSqlDatabase::database().commit();
}
