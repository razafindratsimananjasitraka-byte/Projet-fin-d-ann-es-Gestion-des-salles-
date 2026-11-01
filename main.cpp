#include "database.h"
#include "mainwindow.h"
#include "theme.h"

#include <QApplication>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setOrganizationName(QStringLiteral("GestionSalles"));
    app.setApplicationName(QStringLiteral("GestionSalles"));
    app.setStyle(QStringLiteral("Fusion"));
    app.setStyleSheet(Theme::globalStyle());

    QString error;
    if (!Database::open(&error)) {
        QMessageBox::critical(nullptr, QStringLiteral("Erreur"),
                              QStringLiteral("Impossible d'ouvrir la base de données :\n%1").arg(error));
        return 1;
    }

    MainWindow window;
    window.show();
    return app.exec();
}
