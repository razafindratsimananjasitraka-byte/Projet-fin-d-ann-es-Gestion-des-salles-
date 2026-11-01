#include "theme.h"

namespace Theme {

QString statusColor(const QString &status)
{
    if (status == QLatin1String("Disponible") || status == QLatin1String("Actif")
        || status == QLatin1String("Active")) {
        return kGreen;
    }
    if (status == QLatin1String("Annulée")) {
        return QStringLiteral("#FF5C5C");
    }
    return kAccent;
}

QString globalStyle()
{
    return QString::fromUtf8(R"CSS(
QToolTip { background: #1F2A3C; color: #FFFFFF; border: 1px solid #4A5A78; }

QLineEdit, QSpinBox, QComboBox, QDateEdit, QTimeEdit {
    background: #0A1122;
    border: 1px solid #4A5A78;
    border-radius: 10px;
    padding: 8px 12px;
    min-height: 22px;
    color: #FFFFFF;
    selection-background-color: #FFD500;
    selection-color: #000000;
}
QLineEdit:focus, QSpinBox:focus, QComboBox:focus, QDateEdit:focus, QTimeEdit:focus {
    border: 1px solid #FFD500;
}
QComboBox:disabled { color: #C8CFDB; }
QComboBox QAbstractItemView {
    background: #0A1122;
    color: #FFFFFF;
    border: 1px solid #4A5A78;
    selection-background-color: #FFD500;
    selection-color: #000000;
    outline: 0;
}

QCalendarWidget QWidget { background: #1F2A3C; color: #FFFFFF; }
QCalendarWidget QAbstractItemView:enabled {
    background: #0A1122;
    color: #FFFFFF;
    selection-background-color: #FFD500;
    selection-color: #000000;
}
QCalendarWidget QAbstractItemView:disabled { color: #5B6579; }
QCalendarWidget QToolButton { color: #FFFFFF; background: transparent; }

QMessageBox { background: #1F2A3C; }
QMessageBox QLabel { color: #FFFFFF; font-size: 14px; background: transparent; }

QScrollBar:vertical { background: #0A1122; width: 10px; margin: 0; }
QScrollBar::handle:vertical { background: #4A5A78; border-radius: 5px; min-height: 24px; }
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
QScrollBar:horizontal { background: #0A1122; height: 10px; margin: 0; }
QScrollBar::handle:horizontal { background: #4A5A78; border-radius: 5px; min-width: 24px; }
QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0; }
)CSS");
}

}
