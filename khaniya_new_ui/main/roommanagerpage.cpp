#include "../include/roommanagerpage.h"
#include <sqlite3.h>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QMessageBox>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QDebug>

RoomManagerPage::RoomManagerPage(sqlite3* db, QWidget *parent)
    : QWidget(parent), m_db(db)
{
    if (!m_db) {
        qWarning() << "RoomManagerPage initialized with null database!";
    } else {
        sqlite3_exec(m_db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr);
    }

    m_firstLoad = true;
    buildUi();
    loadData();
    m_firstLoad = false;
}

RoomManagerPage::~RoomManagerPage() {
    // DB is owned by MainWindow, do not close here
}

void RoomManagerPage::buildUi() {
    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(20);

    // Left Panel: Halls
    auto* leftPanel = new QWidget(this);
    auto* leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(0, 0, 0, 0);

    auto* hallTitle = new QLabel("Halls (Exam Venues)", this);
    hallTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #003366;");
    leftLayout->addWidget(hallTitle);

    m_hallTable = new QTableWidget(this);
    m_hallTable->setColumnCount(2);
    m_hallTable->setHorizontalHeaderLabels({"Hall ID", "Hall Name"});
    m_hallTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_hallTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_hallTable->setSelectionMode(QAbstractItemView::SingleSelection);
    connect(m_hallTable, &QTableWidget::cellClicked, this, &RoomManagerPage::onHallSelected);
    leftLayout->addWidget(m_hallTable);

    auto* leftBtns = new QHBoxLayout();
    auto* addHallBtn = new QPushButton("Add Hall", this);
    auto* removeHallBtn = new QPushButton("Remove Selected", this);
    addHallBtn->setStyleSheet("background-color: #003366; color: white; padding: 6px 12px;");
    removeHallBtn->setStyleSheet("background-color: #dc3545; color: white; padding: 6px 12px;");
    connect(addHallBtn, &QPushButton::clicked, this, &RoomManagerPage::onAddHall);
    connect(removeHallBtn, &QPushButton::clicked, this, &RoomManagerPage::onRemoveHall);
    leftBtns->addWidget(addHallBtn);
    leftBtns->addWidget(removeHallBtn);
    leftLayout->addLayout(leftBtns);

    mainLayout->addWidget(leftPanel, 2);

    // Right Panel: Rooms
    auto* rightPanel = new QWidget(this);
    auto* rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(0, 0, 0, 0);

    m_selectedHallLbl = new QLabel("Rooms (Classrooms / Sub-halls) of Selected Hall", this);
    m_selectedHallLbl->setStyleSheet("font-size: 16px; font-weight: bold; color: #003366;");
    rightLayout->addWidget(m_selectedHallLbl);

    m_roomTable = new QTableWidget(this);
    m_roomTable->setColumnCount(7);
    m_roomTable->setHorizontalHeaderLabels({"Room ID", "Room Name", "Rows", "Cols", "Capacity", "Isolated", "Accessible"});
    m_roomTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_roomTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_roomTable->setSelectionMode(QAbstractItemView::SingleSelection);
    rightLayout->addWidget(m_roomTable);

    auto* rightBtns = new QHBoxLayout();
    auto* addRoomBtn = new QPushButton("Add Room", this);
    auto* removeRoomBtn = new QPushButton("Remove Selected", this);
    addRoomBtn->setStyleSheet("background-color: #003366; color: white; padding: 6px 12px;");
    removeRoomBtn->setStyleSheet("background-color: #dc3545; color: white; padding: 6px 12px;");
    connect(addRoomBtn, &QPushButton::clicked, this, &RoomManagerPage::onAddRoom);
    connect(removeRoomBtn, &QPushButton::clicked, this, &RoomManagerPage::onRemoveRoom);
    rightBtns->addWidget(addRoomBtn);
    rightBtns->addWidget(removeRoomBtn);
    rightLayout->addLayout(rightBtns);

    mainLayout->addWidget(rightPanel, 3);
}

void RoomManagerPage::loadData() {
    if (!m_db) return;

    nasala::HallRepository hallRepo(m_db);
    auto allHalls = hallRepo.loadAll();

    m_manager = nasala::HallManager();
    for (const auto& h : allHalls) {
        m_manager.addHall(h);
    }

    // Seed database ONLY on first load if entirely empty
    if (m_firstLoad && m_manager.isEmpty()) {
        nasala::Hall defaultHall("MAIN_HALL", "KU Central Exam Hall");
        nasala::Room defaultRoom("MAIN_ROOM", "Main Hall Seating Room", 18, 18);
        defaultHall.addRoom(defaultRoom);

        hallRepo.save(defaultHall);
        m_manager.addHall(defaultHall);
    }

    populateHalls();
}

void RoomManagerPage::populateHalls(const QString& selectHallID) {
    m_hallTable->setRowCount(0);
    const auto& halls = m_manager.getHalls();
    int selectIdx = 0;
    for (int i = 0; i < halls.size(); ++i) {
        m_hallTable->insertRow(i);
        QString hId = QString::fromStdString(halls[i].getHallID());
        m_hallTable->setItem(i, 0, new QTableWidgetItem(hId));
        m_hallTable->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(halls[i].getHallName())));
        if (!selectHallID.isEmpty() && hId == selectHallID) {
            selectIdx = i;
        }
    }
    if (halls.size() > 0) {
        m_hallTable->selectRow(selectIdx);
        onHallSelected(selectIdx, 0);
    } else {
        m_roomTable->setRowCount(0);
        m_selectedHallLbl->setText("No Hall Selected");
    }
}

void RoomManagerPage::onHallSelected(int row, int) {
    if (row < 0 || row >= m_hallTable->rowCount()) return;
    QString hallID = m_hallTable->item(row, 0)->text();
    QString hallName = m_hallTable->item(row, 1)->text();
    m_selectedHallLbl->setText(QString("Rooms of %1").arg(hallName));
    populateRooms(hallID);
}

void RoomManagerPage::populateRooms(const QString& hallID, const QString& selectRoomID) {
    m_roomTable->setRowCount(0);
    auto* hall = m_manager.findHall(hallID.toStdString());
    if (!hall) return;

    const auto& rooms = hall->getRooms();
    int selectIdx = -1;
    for (int i = 0; i < rooms.size(); ++i) {
        m_roomTable->insertRow(i);
        QString rId = QString::fromStdString(rooms[i].getRoomID());
        m_roomTable->setItem(i, 0, new QTableWidgetItem(rId));
        m_roomTable->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(rooms[i].getRoomName())));
        m_roomTable->setItem(i, 2, new QTableWidgetItem(QString::number(rooms[i].getRows())));
        m_roomTable->setItem(i, 3, new QTableWidgetItem(QString::number(rooms[i].getColumns())));
        m_roomTable->setItem(i, 4, new QTableWidgetItem(QString::number(rooms[i].getCapacity())));
        m_roomTable->setItem(i, 5, new QTableWidgetItem(rooms[i].isIsolated() ? "Yes" : "No"));
        m_roomTable->setItem(i, 6, new QTableWidgetItem(rooms[i].isAccessible() ? "Yes" : "No"));
        if (!selectRoomID.isEmpty() && rId == selectRoomID) {
            selectIdx = i;
        }
    }
    if (selectIdx >= 0) {
        m_roomTable->selectRow(selectIdx);
    } else if (rooms.size() > 0) {
        m_roomTable->selectRow(0);
    }
}

QString RoomManagerPage::selectedHallID() const {
    int row = m_hallTable->currentRow();
    if (row < 0) return "";
    return m_hallTable->item(row, 0)->text();
}

QString RoomManagerPage::selectedRoomID() const {
    int row = m_roomTable->currentRow();
    if (row < 0) return "";
    return m_roomTable->item(row, 0)->text();
}

// ════════════════════════════════════════════════════════════════════════════
// onAddHall — 3-step guided wizard:
//   Step 1: Hall ID + Hall Name + number of sections
//   Step 2: Shared layout config (rows × cols) for all sections
//   Save:   Create one Room per section letter (A, B, C…)
// ════════════════════════════════════════════════════════════════════════════
void RoomManagerPage::onAddHall() {

    // ── STEP 1: Hall Identity & Section Count ──────────────────────────────
    QDialog hallDlg(this);
    hallDlg.setWindowTitle("Add New Exam Hall — Step 1 of 2");
    hallDlg.setMinimumWidth(480);
    hallDlg.setStyleSheet(
        "QDialog { background:#f8fafc; }"
        "QLabel  { color:#0f172a; }"
        "QLineEdit, QSpinBox { border:1.5px solid #cbd5e1; border-radius:6px; "
        "                      padding:7px 10px; font-size:12px; background:white; }"
        "QLineEdit:focus, QSpinBox:focus { border-color:#003366; }"
        "QDialogButtonBox QPushButton { padding:7px 20px; border-radius:6px; }"
    );

    auto *hallVL = new QVBoxLayout(&hallDlg);
    hallVL->setContentsMargins(24, 24, 24, 18);
    hallVL->setSpacing(14);

    auto *hdrLbl = new QLabel("🏛  New Examination Hall", &hallDlg);
    hdrLbl->setStyleSheet("font-size:16px; font-weight:bold; color:#003366; margin-bottom:4px;");
    hallVL->addWidget(hdrLbl);

    auto *hallForm = new QFormLayout;
    hallForm->setSpacing(10);

    auto *hallIdEdit   = new QLineEdit(&hallDlg);
    auto *hallNameEdit = new QLineEdit(&hallDlg);
    hallIdEdit->setPlaceholderText("e.g. HALL_B  (no spaces)");
    hallNameEdit->setPlaceholderText("e.g. Engineering Block Hall");

    auto *sectionSpin = new QSpinBox(&hallDlg);
    sectionSpin->setRange(0, 26);
    sectionSpin->setValue(1);
    sectionSpin->setSpecialValueText("0  —  Single room (no sections)");
    sectionSpin->setSuffix("  section(s)  [A, B, C …]");

    hallForm->addRow("Hall ID:", hallIdEdit);
    hallForm->addRow("Hall Name:", hallNameEdit);
    hallForm->addRow("Number of Sections:", sectionSpin);

    auto *sectionHint = new QLabel(
        "Each section becomes a separately labelled seating area (A, B, C…).\n"
        "Set to 0 for a single unnamed room.",
        &hallDlg);
    sectionHint->setStyleSheet("font-size:11px; color:#64748b;");
    sectionHint->setWordWrap(true);

    // Live preview of section letters
    auto *previewLbl = new QLabel(&hallDlg);
    previewLbl->setStyleSheet("font-size:12px; color:#003366; font-weight:600; min-height:20px;");
    auto updatePreview = [&]() {
        int n = sectionSpin->value();
        if (n == 0) { previewLbl->setText("No sections — one room total."); return; }
        QStringList letters;
        for (int i = 0; i < n && i < 26; ++i) letters << QString(QChar('A' + i));
        previewLbl->setText(QString("Sections: %1").arg(letters.join("  ")));
    };
    connect(sectionSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [updatePreview](int){ updatePreview(); });
    updatePreview();

    hallVL->addLayout(hallForm);
    hallVL->addWidget(sectionHint);
    hallVL->addWidget(previewLbl);

    auto *hallBbox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &hallDlg);
    hallVL->addWidget(hallBbox);
    connect(hallBbox, &QDialogButtonBox::accepted, &hallDlg, &QDialog::accept);
    connect(hallBbox, &QDialogButtonBox::rejected, &hallDlg, &QDialog::reject);

    if (hallDlg.exec() != QDialog::Accepted) return;

    QString id   = hallIdEdit->text().trimmed().replace(" ", "_").toUpper();
    QString name = hallNameEdit->text().trimmed();
    int numSections = sectionSpin->value();

    if (id.isEmpty() || name.isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Hall ID and Hall Name are required.");
        return;
    }
    if (m_manager.findHall(id.toStdString())) {
        QMessageBox::warning(this, "Duplicate ID",
            QString("A Hall with ID \"%1\" already exists. Please choose a different ID.").arg(id));
        return;
    }

    // ── STEP 2: Layout Config ─────────────────────────────────────────────
    QString step2Title = numSections > 0
        ? QString("Add New Exam Hall — Step 2 of 2: Section Layout")
        : "Add New Exam Hall — Step 2 of 2: Room Layout";

    QDialog layoutDlg(this);
    layoutDlg.setWindowTitle(step2Title);
    layoutDlg.setMinimumWidth(500);
    layoutDlg.setStyleSheet(hallDlg.styleSheet());

    auto *layVL = new QVBoxLayout(&layoutDlg);
    layVL->setContentsMargins(24, 24, 24, 18);
    layVL->setSpacing(14);

    QString layHdrTxt = numSections > 0
        ? "📐  Section Layout Configuration"
        : "📐  Room Layout Configuration";
    auto *layHdr = new QLabel(layHdrTxt, &layoutDlg);
    layHdr->setStyleSheet("font-size:15px; font-weight:bold; color:#003366; margin-bottom:4px;");
    layVL->addWidget(layHdr);

    if (numSections > 0) {
        QStringList letters;
        for (int i = 0; i < numSections && i < 26; ++i) letters << QString(QChar('A' + i));
        auto *secLbl = new QLabel(
            QString("The layout below applies to every section equally.\n"
                    "Sections to create:  <b>%1</b>").arg(letters.join(",  ")),
            &layoutDlg);
        secLbl->setStyleSheet("font-size:12px; color:#1e293b;");
        secLbl->setWordWrap(true);
        layVL->addWidget(secLbl);
    }

    auto *layForm = new QFormLayout;
    layForm->setSpacing(10);

    auto *rowsSpin = new QSpinBox(&layoutDlg);
    rowsSpin->setRange(1, 200); rowsSpin->setValue(10); rowsSpin->setSuffix(" rows");

    auto *colsSpin = new QSpinBox(&layoutDlg);
    colsSpin->setRange(1, 200); colsSpin->setValue(10); colsSpin->setSuffix(" seats per row");

    auto *isolatedCheck   = new QCheckBox("Mark as isolated classroom", &layoutDlg);
    auto *accessibleCheck = new QCheckBox("Mark as physically accessible", &layoutDlg);

    auto *capLbl = new QLabel(&layoutDlg);
    capLbl->setStyleSheet("font-size:12px; font-weight:bold; color:#003366; padding:4px 0;");
    capLbl->setWordWrap(true);
    auto updateCap = [&]() {
        int perSec = rowsSpin->value() * colsSpin->value();
        int total  = numSections > 0 ? perSec * numSections : perSec;
        if (numSections > 0)
            capLbl->setText(QString("Per section: %1 seats  ·  Total hall capacity: %2 seats").arg(perSec).arg(total));
        else
            capLbl->setText(QString("Total capacity: %1 seats").arg(perSec));
    };
    connect(rowsSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [updateCap](int){ updateCap(); });
    connect(colsSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [updateCap](int){ updateCap(); });
    updateCap();

    // Seat code preview
    auto *seatCodeLbl = new QLabel(&layoutDlg);
    seatCodeLbl->setStyleSheet("font-size:11px; color:#64748b;");
    auto updateCodePreview = [&]() {
        if (numSections > 0)
            seatCodeLbl->setText("Seat code format:  Section-Row-Col  (e.g. A-A-1, B-C-5)");
        else
            seatCodeLbl->setText("Seat code format:  RoomID-Row-Col  (e.g. HALL_ROOM-A-1)");
    };
    updateCodePreview();

    layForm->addRow("Rows:", rowsSpin);
    layForm->addRow("Seats per row (Columns):", colsSpin);
    layForm->addRow("", capLbl);
    layForm->addRow("", seatCodeLbl);
    layForm->addRow(isolatedCheck);
    layForm->addRow(accessibleCheck);
    layVL->addLayout(layForm);

    auto *layBbox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &layoutDlg);
    layVL->addWidget(layBbox);
    connect(layBbox, &QDialogButtonBox::accepted, &layoutDlg, &QDialog::accept);
    connect(layBbox, &QDialogButtonBox::rejected, &layoutDlg, &QDialog::reject);

    if (layoutDlg.exec() != QDialog::Accepted) return;

    int rows       = rowsSpin->value();
    int cols       = colsSpin->value();
    bool isolated  = isolatedCheck->isChecked();
    bool accessible= accessibleCheck->isChecked();

    // ── Build Rooms ───────────────────────────────────────────────────────
    nasala::Hall newHall(id.toStdString(), name.toStdString());

    if (numSections > 0) {
        for (int i = 0; i < numSections && i < 26; ++i) {
            QString sec = QString(QChar('A' + i));
            // Room ID  = HALL_A, HALL_B, …
            // Room Name= "Engineering Block Hall — Section A"
            // The section letter becomes the venue code (seat prefix) automatically
            // because HallModel::setActiveRoom derives venueCode from the room name
            // and our seatingpage passes the section letter explicitly.
            std::string roomID   = id.toStdString() + "_" + sec.toStdString();
            std::string roomName = name.toStdString() + " \xe2\x80\x94 Section " + sec.toStdString();
            newHall.addRoom(nasala::Room(roomID, roomName, rows, cols, isolated, accessible));
        }
    } else {
        std::string roomID   = id.toStdString() + "_ROOM";
        std::string roomName = name.toStdString() + " Room";
        newHall.addRoom(nasala::Room(roomID, roomName, rows, cols, isolated, accessible));
    }

    // ── Save ─────────────────────────────────────────────────────────────
    nasala::HallRepository hallRepo(m_db);
    if (hallRepo.save(newHall)) {
        m_manager.addHall(newHall);
        populateHalls(id);
        emit roomChanged();

        // Summary message
        QString summary;
        if (numSections > 0) {
            QStringList lines;
            for (int i = 0; i < numSections && i < 26; ++i)
                lines << QString("  Section %1  (%2 rows × %3 seats = %4 seats)")
                         .arg(QChar('A'+i)).arg(rows).arg(cols).arg(rows*cols);
            summary = "\n\nSections created:\n" + lines.join("\n");
        } else {
            summary = QString("\n\nSingle room: %1 rows × %2 seats = %3 seats total")
                .arg(rows).arg(cols).arg(rows*cols);
        }
        QMessageBox::information(this, "Hall Added",
            QString("Hall \"%1\" saved successfully.%2").arg(name).arg(summary));
    } else {
        QMessageBox::critical(this, "Database Error", "Failed to save the Hall to the database.");
    }
}

void RoomManagerPage::onRemoveHall() {
    QString id = selectedHallID();
    if (id.isEmpty()) return;

    auto confirm = QMessageBox::question(this, "Confirm Delete",
        QString("Are you sure you want to delete Hall \"%1\"?\nAll its rooms will also be deleted.").arg(id));
    if (confirm != QMessageBox::Yes) return;

    nasala::HallRepository hallRepo(m_db);
    if (hallRepo.remove(id.toStdString())) {
        m_manager.removeHall(id.toStdString());
        populateHalls();
        emit roomChanged();
    } else {
        QMessageBox::critical(this, "Error", "Failed to remove Hall from Database.");
    }
}

void RoomManagerPage::onAddRoom() {
    QString hallID = selectedHallID();
    if (hallID.isEmpty()) return;

    QDialog dialog(this);
    dialog.setWindowTitle("Add Room to Hall");
    auto* form = new QFormLayout(&dialog);
    form->setContentsMargins(20, 20, 20, 16);
    form->setSpacing(10);

    auto* idEdit = new QLineEdit(&dialog);
    auto* nameEdit = new QLineEdit(&dialog);
    auto* rowsSpin = new QSpinBox(&dialog);
    rowsSpin->setRange(1, 200); rowsSpin->setValue(10); rowsSpin->setSuffix(" rows");
    auto* colsSpin = new QSpinBox(&dialog);
    colsSpin->setRange(1, 200); colsSpin->setValue(10); colsSpin->setSuffix(" seats per row");

    auto* isolatedCheck  = new QCheckBox("Mark as isolated classroom", &dialog);
    auto* accessibleCheck= new QCheckBox("Mark as physically accessible", &dialog);

    auto* capLbl = new QLabel(&dialog);
    capLbl->setStyleSheet("font-weight:bold; color:#003366;");
    auto updateCap = [&]() {
        capLbl->setText(QString("Capacity: %1 seats").arg(rowsSpin->value() * colsSpin->value()));
    };
    connect(rowsSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [updateCap](int){ updateCap(); });
    connect(colsSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [updateCap](int){ updateCap(); });
    updateCap();

    form->addRow("Room ID:", idEdit);
    form->addRow("Room Name:", nameEdit);
    form->addRow("Rows:", rowsSpin);
    form->addRow("Seats per row:", colsSpin);
    form->addRow("", capLbl);
    form->addRow(isolatedCheck);
    form->addRow(accessibleCheck);

    auto* bbox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    form->addRow(bbox);
    connect(bbox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(bbox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted) return;

    QString roomID   = idEdit->text().trimmed();
    QString roomName = nameEdit->text().trimmed();
    if (roomID.isEmpty() || roomName.isEmpty()) {
        QMessageBox::warning(this, "Error", "All fields are required.");
        return;
    }

    auto* hall = m_manager.findHall(hallID.toStdString());
    if (!hall) return;
    if (hall->findRoom(roomID.toStdString())) {
        QMessageBox::warning(this, "Error", "Room ID already exists in this Hall.");
        return;
    }

    nasala::Room newRoom(roomID.toStdString(), roomName.toStdString(),
                         rowsSpin->value(), colsSpin->value(),
                         isolatedCheck->isChecked(), accessibleCheck->isChecked());

    nasala::RoomRepository roomRepo(m_db);
    if (roomRepo.save(newRoom, hallID.toStdString())) {
        hall->addRoom(newRoom);
        populateRooms(hallID, roomID);
        emit roomChanged();
    } else {
        QMessageBox::critical(this, "Error", "Failed to save Room to Database.");
    }
}

void RoomManagerPage::onRemoveRoom() {
    QString hallID = selectedHallID();
    QString roomID = selectedRoomID();
    if (hallID.isEmpty() || roomID.isEmpty()) return;

    auto confirm = QMessageBox::question(this, "Confirm Delete",
        QString("Are you sure you want to delete Room \"%1\"?").arg(roomID));
    if (confirm != QMessageBox::Yes) return;

    nasala::RoomRepository roomRepo(m_db);
    if (roomRepo.remove(roomID.toStdString())) {
        auto* hall = m_manager.findHall(hallID.toStdString());
        if (hall) hall->removeRoom(roomID.toStdString());
        populateRooms(hallID);
        emit roomChanged();
    } else {
        QMessageBox::critical(this, "Error", "Failed to remove Room from Database.");
    }
}

void RoomManagerPage::refresh() {
    loadData();
}
