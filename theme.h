#pragma once

#include <QString>

namespace Theme {

inline const QString kBackground = QStringLiteral("#0A1122");
inline const QString kPanel      = QStringLiteral("#1F2A3C");
inline const QString kBorder     = QStringLiteral("#4A5A78");
inline const QString kAccent     = QStringLiteral("#FFD500");
inline const QString kGreen      = QStringLiteral("#00B050");
inline const QString kBlue       = QStringLiteral("#00B0F0");
inline const QString kRed        = QStringLiteral("#FF2A2A");
inline const QString kMuted      = QStringLiteral("#8A94A6");

QString statusColor(const QString &status);
QString globalStyle();

}

namespace Schedule {

constexpr int kFirstHour = 7;
constexpr int kLastHour = 19;

}
