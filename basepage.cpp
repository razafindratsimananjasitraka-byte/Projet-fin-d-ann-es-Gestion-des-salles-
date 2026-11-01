#include "basepage.h"

#include "theme.h"
#include "widgets.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QTimer>
#include <QVBoxLayout>

namespace {

constexpr int kRowsPerPage = 8;
constexpr int kRowHeight = 48;

}

BasePage::BasePage(const PageConfig &config, QWidget *parent)
    : QWidget(parent), m_config(config)
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(32, 12, 32, 20);
    root->setSpacing(14);

    auto *header = new PageHeader(config.crumb, config.titleLead, config.titleAccent,
                                  config.subtitle, this);
    connect(header, &PageHeader::homeRequested, this, &BasePage::homeRequested);
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
    m_search = new QLineEdit(this);
    m_search->setPlaceholderText(QStringLiteral("Recherche…"));
    m_filter = new QComboBox(this);
    m_filter->addItem(QStringLiteral("Tous"));
    m_filter->addItems(config.filters);
    m_filter->setFixedWidth(150);
    QPushButton *addButton = makeButton(config.addLabel, Theme::kAccent, QStringLiteral("#000000"), this);
    addButton->setMinimumHeight(40);
    toolbar->addWidget(m_search, 1);
    toolbar->addWidget(m_filter);
    toolbar->addWidget(addButton);
    root->addLayout(toolbar);

    QStringList headers;
    headers << QStringLiteral("N°") << config.headers << QStringLiteral("Actions");
    m_table = new QTableWidget(0, headers.size(), this);
    m_table->setHorizontalHeaderLabels(headers);
    m_table->verticalHeader()->setVisible(false);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionMode(QAbstractItemView::NoSelection);
    m_table->setFocusPolicy(Qt::NoFocus);
    m_table->setShowGrid(false);
    m_table->setWordWrap(false);

    QHeaderView *horizontal = m_table->horizontalHeader();
    horizontal->setSectionResizeMode(QHeaderView::Stretch);
    horizontal->setSectionResizeMode(0, QHeaderView::Fixed);
    horizontal->setSectionResizeMode(headers.size() - 1, QHeaderView::Fixed);
    horizontal->setHighlightSections(false);
    horizontal->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_table->setColumnWidth(0, 64);
    m_table->setColumnWidth(headers.size() - 1, 300);

    m_table->setStyleSheet(QStringLiteral(
        "QTableWidget { background: %1; border: 1px solid %2; border-radius: 12px; outline: 0; }"
        "QTableWidget::item { padding-left: 10px; border-bottom: 1px solid #2A3650; }"
        "QHeaderView::section { background: %3; color: #FFFFFF; padding: 12px 10px; border: none;"
        " font-weight: 600; }"
        "QTableCornerButton::section { background: %3; border: none; }")
                               .arg(Theme::kBackground, Theme::kBorder, Theme::kPanel));
    root->addWidget(m_table, 1);

    auto *pagination = new QHBoxLayout;
    pagination->setSpacing(12);
    pagination->addStretch();
    m_previous = makeButton(QStringLiteral("Précédent"), Theme::kPanel, QStringLiteral("#FFFFFF"), this);
    m_next = makeButton(QStringLiteral("Suivant"), Theme::kPanel, QStringLiteral("#FFFFFF"), this);
    m_pageLabel = new QLabel(this);
    m_pageLabel->setStyleSheet(QStringLiteral("color: #FFFFFF; font-weight: 600;"));
    pagination->addWidget(m_previous);
    pagination->addWidget(m_pageLabel);
    pagination->addWidget(m_next);
    root->addLayout(pagination);

    connect(m_search, &QLineEdit::textChanged, this, [this] {
        m_page = 0;
        applyFilters();
    });
    connect(m_filter, &QComboBox::currentTextChanged, this, [this] {
        m_page = 0;
        applyFilters();
    });
    connect(m_previous, &QPushButton::clicked, this, [this] {
        --m_page;
        applyFilters();
    });
    connect(m_next, &QPushButton::clicked, this, [this] {
        ++m_page;
        applyFilters();
    });
    connect(addButton, &QPushButton::clicked, this, [this] {
        QTimer::singleShot(0, this, [this] {
            addRecord();
            reload();
        });
    });
}

void BasePage::setSearchText(const QString &text)
{
    m_search->setText(text);
}

void BasePage::reload()
{
    m_all = loadRows();
    const QVector<int> values = loadStats();
    for (int i = 0; i < 3 && i < values.size() && i < m_config.statLabels.size(); ++i) {
        m_stats[i]->setContent(m_config.statLabels.at(i), values.at(i));
    }
    applyFilters();
}

void BasePage::applyFilters()
{
    const QString needle = m_search->text().trimmed();
    const bool anyStatus = m_filter->currentIndex() <= 0;
    const QString wanted = m_filter->currentText();

    m_filtered.clear();
    for (const RowData &row : m_all) {
        if (!anyStatus) {
            if (m_config.statusColumn < 0 || row.cells.value(m_config.statusColumn) != wanted) {
                continue;
            }
        }
        if (!needle.isEmpty()) {
            bool match = false;
            for (const QString &cell : row.cells) {
                if (cell.contains(needle, Qt::CaseInsensitive)) {
                    match = true;
                    break;
                }
            }
            if (!match) {
                continue;
            }
        }
        m_filtered.append(row);
    }

    const int lastPage = qMax(0, (m_filtered.size() - 1) / kRowsPerPage);
    m_page = qBound(0, m_page, lastPage);
    render();
}

void BasePage::render()
{
    const int start = m_page * kRowsPerPage;
    const int end = qMin(start + kRowsPerPage, m_filtered.size());
    const int actionColumn = m_table->columnCount() - 1;

    m_table->setRowCount(0);
    m_table->setRowCount(qMax(0, end - start));

    for (int i = start; i < end; ++i) {
        const int tableRow = i - start;
        const RowData &row = m_filtered.at(i);

        auto *number = new QTableWidgetItem(QString::number(i + 1));
        number->setFlags(Qt::ItemIsEnabled);
        m_table->setItem(tableRow, 0, number);

        for (int c = 0; c < row.cells.size(); ++c) {
            auto *item = new QTableWidgetItem(row.cells.at(c));
            item->setFlags(Qt::ItemIsEnabled);
            if (c == m_config.statusColumn) {
                item->setForeground(QColor(Theme::statusColor(row.cells.at(c))));
                QFont font = item->font();
                font.setBold(true);
                item->setFont(font);
            }
            m_table->setItem(tableRow, c + 1, item);
        }

        m_table->setCellWidget(tableRow, actionColumn, buildActions(row.id));
        m_table->setRowHeight(tableRow, kRowHeight);
    }

    const int pages = qMax(1, (m_filtered.size() + kRowsPerPage - 1) / kRowsPerPage);
    m_pageLabel->setText(QStringLiteral("Page %1 / %2").arg(m_page + 1).arg(pages));
    m_previous->setEnabled(m_page > 0);
    m_next->setEnabled(m_page + 1 < pages);
}

QWidget *BasePage::buildActions(int id)
{
    auto *holder = new QWidget;
    holder->setObjectName(QStringLiteral("actionsHolder"));
    holder->setStyleSheet(QStringLiteral("QWidget#actionsHolder { background: transparent; }"));
    auto *layout = new QHBoxLayout(holder);
    layout->setContentsMargins(6, 0, 6, 0);
    layout->setSpacing(8);

    QPushButton *view = makeButton(QStringLiteral("Voir"), Theme::kGreen, QStringLiteral("#FFFFFF"), holder);
    QPushButton *edit = makeButton(QStringLiteral("Modifier"), Theme::kBlue, QStringLiteral("#FFFFFF"), holder);
    QPushButton *remove = makeButton(QStringLiteral("Supprimer"), Theme::kRed, QStringLiteral("#FFFFFF"), holder);
    for (QPushButton *button : {view, edit, remove}) {
        button->setFixedHeight(30);
        layout->addWidget(button);
    }

    connect(view, &QPushButton::clicked, this, [this, id] {
        QTimer::singleShot(0, this, [this, id] { viewRecord(id); });
    });
    connect(edit, &QPushButton::clicked, this, [this, id] {
        QTimer::singleShot(0, this, [this, id] {
            editRecord(id);
            reload();
        });
    });
    connect(remove, &QPushButton::clicked, this, [this, id] {
        QTimer::singleShot(0, this, [this, id] { requestDelete(id); });
    });
    return holder;
}

void BasePage::requestDelete(int id)
{
    if (!confirmDialog(this, QStringLiteral("Suppression"), deleteMessage(), QStringLiteral("Supprimer"))) {
        return;
    }
    QString error;
    if (!deleteRecord(id, &error)) {
        infoDialog(this, QStringLiteral("Suppression impossible"), error);
        return;
    }
    reload();
}
