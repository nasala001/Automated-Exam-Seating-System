#include "roompage.h"
#include "mainwindow.h"
#include "Venue.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QFrame>
#include <QLabel>
#include <QTabWidget>

#include <memory>

// ── Shared style strings ───────────────────────────────────────────────────

static const QString kBtnStyle =
    "QPushButton { background: #313244; color: #cdd6f4;"
    "border-radius: 6px; padding: 8px 16px; font-size: 13px; }"
    "QPushButton:hover { background: #45475a; }";

static const QString kAccentBtnStyle =
    "QPushButton { background: #89b4fa; color: #1e1e2e;"
    "border-radius: 6px; padding: 8px 18px; font-weight: bold; font-size: 13px; }"
    "QPushButton:hover { background: #b4d0f7; }";

static const QString kTableStyle =
    "QTableWidget { background: #1e1e2e; color: #cdd6f4;"
    "  gridline-color: #313244; border: none; font-size: 13px; }"
    "QHeaderView::section { background: #313244; color: #89b4fa;"
    "  padding: 8px; border: none; font-weight: bold; }"
    "QTableWidget::item:selected { background: #45475a; }";

static const QString kInputStyle =
    "QLineEdit { background: #313244; color: #cdd6f4;"
    "border: 1px solid #45475a; border-radius: 6px; padding: 6px 10px; }";

static const QString kSpinStyle =
    "QSpinBox { background: #313244; color: #cdd6f4;"
    "border: 1px solid #45475a; border-radius: 6px; padding: 6px 10px; }";

// ── Constructor ────────────────────────────────────────────────────────────

RoomPage::RoomPage(MainWindow *mainWin, QWidget *parent)
    : QWidget(parent), mainWindow(mainWin)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(30, 30, 30, 30);
    layout->setSpacing(14);

    QLabel *title = new QLabel("Rooms & Halls", this);
    title->setStyleSheet("font-size: 26px; font-weight: bold; color: #cdd6f4;");
    layout->addWidget(title);

    // ── Tab widget ─────────────────────
    QTabWidget *tabs = new QTabWidget(this);
    tabs->setStyleSheet(
        "QTabWidget::pane { border: 1px solid #313244; background: #181825; }"
        "QTabBar::tab { background: #313244; color: #cdd6f4; padding: 10px 20px;"
        "  border-top-left-radius: 6px; border-top-right-radius: 6px; }"
        "QTabBar::tab:selected { background: #45475a; color: #89b4fa; font-weight: bold; }"
        "QTabBar::tab:hover { background: #45475a; }"
        );

    tabs->addTab(createHallTab(), "🏛  Large Hall");
    tabs->addTab(createRoomTab(), "🏫  Exam Room");
    layout->addWidget(tabs);

    // ── Venue summary ──────────────────
    QLabel *summaryTitle = new QLabel("All Venues", this);
    summaryTitle->setStyleSheet("font-size: 18px; font-weight: bold; color: #cdd6f4; margin-top: 10px;");
    layout->addWidget(summaryTitle);

    QHBoxLayout *summaryToolbar = new QHBoxLayout();
    QPushButton *btnDeleteVenue = new QPushButton("🗑 Remove Venue", this);
    btnDeleteVenue->setStyleSheet(kBtnStyle);
    summaryToolbar->addWidget(btnDeleteVenue);
    summaryToolbar->addStretch();
    layout->addLayout(summaryToolbar);

    venueSummaryTable = new QTableWidget(0, 3, this);
    venueSummaryTable->setHorizontalHeaderLabels({"Name", "Type", "Capacity"});
    venueSummaryTable->horizontalHeader()->setStretchLastSection(true);
    venueSummaryTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    venueSummaryTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    venueSummaryTable->setStyleSheet(kTableStyle);
    layout->addWidget(venueSummaryTable);

    setLayout(layout);

    connect(btnDeleteVenue, &QPushButton::clicked, this, &RoomPage::deleteVenue);
}

// ── Hall Tab ───────────────────────────────────────────────────────────────

QWidget* RoomPage::createHallTab()
{
    QWidget *tab = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(tab);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(10);

    // Hall name + rows
    QHBoxLayout *topRow = new QHBoxLayout();

    QLabel *nameLabel = new QLabel("Hall Name:", tab);
    nameLabel->setStyleSheet("color: #cdd6f4;");
    hallNameEdit = new QLineEdit(tab);
    hallNameEdit->setPlaceholderText("e.g. Multipurpose Hall");
    hallNameEdit->setStyleSheet(kInputStyle);

    QLabel *rowsLabel = new QLabel("Rows:", tab);
    rowsLabel->setStyleSheet("color: #cdd6f4;");
    hallRowsSpin = new QSpinBox(tab);
    hallRowsSpin->setRange(1, 100);
    hallRowsSpin->setValue(10);
    hallRowsSpin->setStyleSheet(kSpinStyle);

    topRow->addWidget(nameLabel);
    topRow->addWidget(hallNameEdit);
    topRow->addWidget(rowsLabel);
    topRow->addWidget(hallRowsSpin);
    layout->addLayout(topRow);

    // Section table
    QLabel *secLabel = new QLabel("Sections:", tab);
    secLabel->setStyleSheet("color: #a6adc8; font-size: 13px;");
    layout->addWidget(secLabel);

    sectionTable = new QTableWidget(0, 2, tab);
    sectionTable->setHorizontalHeaderLabels({"Label (e.g. A)", "Columns"});
    sectionTable->horizontalHeader()->setStretchLastSection(true);
    sectionTable->setStyleSheet(kTableStyle);
    sectionTable->setMaximumHeight(150);
    layout->addWidget(sectionTable);

    // Section buttons
    QHBoxLayout *secBtns = new QHBoxLayout();
    QPushButton *btnAddSec = new QPushButton("+ Add Section", tab);
    QPushButton *btnRemSec = new QPushButton("- Remove Section", tab);
    btnAddSec->setStyleSheet(kBtnStyle);
    btnRemSec->setStyleSheet(kBtnStyle);
    secBtns->addWidget(btnAddSec);
    secBtns->addWidget(btnRemSec);
    secBtns->addStretch();
    layout->addLayout(secBtns);

    // Save hall button
    QPushButton *btnSaveHall = new QPushButton("✅ Save Hall", tab);
    btnSaveHall->setStyleSheet(kAccentBtnStyle);
    layout->addWidget(btnSaveHall);

    layout->addStretch();

    connect(btnAddSec,    &QPushButton::clicked, this, &RoomPage::addSection);
    connect(btnRemSec,    &QPushButton::clicked, this, &RoomPage::removeSection);
    connect(btnSaveHall,  &QPushButton::clicked, this, &RoomPage::saveHall);

    return tab;
}

// ── Room Tab ───────────────────────────────────────────────────────────────

QWidget* RoomPage::createRoomTab()
{
    QWidget *tab = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(tab);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(10);

    // Block name + room number
    QHBoxLayout *row1 = new QHBoxLayout();
    QLabel *blockLabel = new QLabel("Block Name:", tab);
    blockLabel->setStyleSheet("color: #cdd6f4;");
    blockNameEdit = new QLineEdit(tab);
    blockNameEdit->setPlaceholderText("e.g. Block 8");
    blockNameEdit->setStyleSheet(kInputStyle);

    QLabel *roomLabel = new QLabel("Room No:", tab);
    roomLabel->setStyleSheet("color: #cdd6f4;");
    roomNumberEdit = new QLineEdit(tab);
    roomNumberEdit->setPlaceholderText("e.g. 304");
    roomNumberEdit->setStyleSheet(kInputStyle);

    row1->addWidget(blockLabel);
    row1->addWidget(blockNameEdit);
    row1->addWidget(roomLabel);
    row1->addWidget(roomNumberEdit);
    layout->addLayout(row1);

    // Rows + cols
    QHBoxLayout *row2 = new QHBoxLayout();
    QLabel *rowsLabel = new QLabel("Rows:", tab);
    rowsLabel->setStyleSheet("color: #cdd6f4;");
    roomRowsSpin = new QSpinBox(tab);
    roomRowsSpin->setRange(1, 50);
    roomRowsSpin->setValue(5);
    roomRowsSpin->setStyleSheet(kSpinStyle);

    QLabel *colsLabel = new QLabel("Cols:", tab);
    colsLabel->setStyleSheet("color: #cdd6f4;");
    roomColsSpin = new QSpinBox(tab);
    roomColsSpin->setRange(1, 50);
    roomColsSpin->setValue(6);
    roomColsSpin->setStyleSheet(kSpinStyle);

    row2->addWidget(rowsLabel);
    row2->addWidget(roomRowsSpin);
    row2->addWidget(colsLabel);
    row2->addWidget(roomColsSpin);
    row2->addStretch();
    layout->addLayout(row2);

    // Add room button
    QPushButton *btnAddRoom = new QPushButton("✅ Add Room", tab);
    btnAddRoom->setStyleSheet(kAccentBtnStyle);
    layout->addWidget(btnAddRoom);

    layout->addStretch();

    connect(btnAddRoom, &QPushButton::clicked, this, &RoomPage::addRoom);

    return tab;
}

// ── Slot implementations ───────────────────────────────────────────────────

void RoomPage::addSection()
{
    int row = sectionTable->rowCount();
    sectionTable->insertRow(row);

    // Default label: next letter (A, B, C, ...)
    QString label = QString(QChar('A' + row));
    QTableWidgetItem *labelItem = new QTableWidgetItem(label);
    QTableWidgetItem *colsItem  = new QTableWidgetItem("3");
    sectionTable->setItem(row, 0, labelItem);
    sectionTable->setItem(row, 1, colsItem);
}

void RoomPage::removeSection()
{
    int row = sectionTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "No Selection", "Please select a section to remove.");
        return;
    }
    sectionTable->removeRow(row);
}

void RoomPage::saveHall()
{
    QString hallName = hallNameEdit->text().trimmed();
    if (hallName.isEmpty()) {
        QMessageBox::warning(this, "Missing Info", "Please enter a hall name.");
        return;
    }

    int hallRows = hallRowsSpin->value();
    int sectionCount = sectionTable->rowCount();
    if (sectionCount == 0) {
        QMessageBox::warning(this, "No Sections", "Please add at least one section.");
        return;
    }

    // Build sections vector
    std::vector<Section> sections;
    for (int i = 0; i < sectionCount; i++) {
        QString label = sectionTable->item(i, 0)->text().trimmed();
        int cols = sectionTable->item(i, 1)->text().toInt();
        if (label.isEmpty() || cols <= 0) {
            QMessageBox::warning(this, "Invalid Section",
                                 QString("Section at row %1 has invalid data.").arg(i + 1));
            return;
        }
        sections.push_back({label.toStdString(), cols});
    }

    // Create LargeHall and generate layout
    auto hall = std::make_unique<LargeHall>(hallName.toStdString(), sections, hallRows);
    hall->generateLayout();

    int capacity = hall->totalSeats();
    mainWindow->addVenue(std::move(hall));

    refreshVenueSummary();

    QMessageBox::information(this, "Hall Saved",
                             QString("'%1' added with %2 sections, %3 total seats.")
                                 .arg(hallName)
                                 .arg(sectionCount)
                                 .arg(capacity));

    // Clear form
    hallNameEdit->clear();
    sectionTable->setRowCount(0);
}

void RoomPage::addRoom()
{
    QString blockName  = blockNameEdit->text().trimmed();
    QString roomNumber = roomNumberEdit->text().trimmed();

    if (blockName.isEmpty() || roomNumber.isEmpty()) {
        QMessageBox::warning(this, "Missing Info",
                             "Please enter both Block Name and Room Number.");
        return;
    }

    int rows = roomRowsSpin->value();
    int cols = roomColsSpin->value();

    auto room = std::make_unique<ExamRoom>(blockName.toStdString(),
                                           roomNumber.toStdString(),
                                           rows, cols);
    room->generateLayout();

    int capacity = room->totalSeats();
    mainWindow->addVenue(std::move(room));

    refreshVenueSummary();

    QMessageBox::information(this, "Room Added",
                             QString("'%1 - %2' added with %3 seats (%4×%5).")
                                 .arg(blockName, roomNumber)
                                 .arg(capacity)
                                 .arg(rows).arg(cols));

    // Clear form
    blockNameEdit->clear();
    roomNumberEdit->clear();
}

void RoomPage::deleteVenue()
{
    int row = venueSummaryTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "No Selection", "Please select a venue to remove.");
        return;
    }

    auto& venues = mainWindow->getVenues();
    if (row < static_cast<int>(venues.size())) {
        venues.erase(venues.begin() + row);
    }

    refreshVenueSummary();
    mainWindow->refreshDashboard();
}

void RoomPage::refreshVenueSummary()
{
    venueSummaryTable->setRowCount(0);
    auto& venues = mainWindow->getVenues();

    for (size_t i = 0; i < venues.size(); i++) {
        int row = venueSummaryTable->rowCount();
        venueSummaryTable->insertRow(row);

        QString name = QString::fromStdString(venues[i]->name);
        QString type = dynamic_cast<LargeHall*>(venues[i].get()) ? "Hall" : "Room";
        QString cap  = QString::number(venues[i]->totalSeats());

        venueSummaryTable->setItem(row, 0, new QTableWidgetItem(name));
        venueSummaryTable->setItem(row, 1, new QTableWidgetItem(type));
        venueSummaryTable->setItem(row, 2, new QTableWidgetItem(cap));
    }
}