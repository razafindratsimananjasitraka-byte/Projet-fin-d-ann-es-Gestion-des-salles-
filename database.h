#pragma once

#include <QSet>
#include <QString>
#include <QVariantList>
#include <QVector>

class Database
{
public:
    static bool open(QString *error);

    static bool exec(const QString &sql, const QVariantList &args = QVariantList(),
                     QString *error = nullptr);
    static QVector<QVariantList> rows(const QString &sql,
                                      const QVariantList &args = QVariantList());
    static int scalar(const QString &sql, const QVariantList &args = QVariantList());

    // Returns an empty string when the slot is free, otherwise a message describing the conflict.
    static QString findConflict(int roomId, int teacherId, const QString &date,
                                const QString &start, const QString &end, int excludeId);

    static QSet<int> conflictingIds();

    static QString displayDate(const QString &isoDate);

private:
    static bool createSchema(QString *error);
    static bool seed(QString *error);
};
