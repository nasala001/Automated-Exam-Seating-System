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
        // Ensure foreign keys are enabled for cascading deletes
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

void RoomManagerPage::onAddHall() {
    bool ok;
    QString id = QInputDialog::getText(this, "Add Hall", "Enter Hall ID (e.g. HALL_B):", QLineEdit::Normal, "", &ok);
    if (!ok || id.trimmed().isEmpty()) return;

    QString name = QInputDialog::getText(this, "Add Hall", "Enter Hall Name:", QLineEdit::Normal, "", &ok);
    if (!ok || name.trimmed().isEmpty()) return;

    if (m_manager.findHall(id.toStdString())) {
        QMessageBox::warning(this, "Error", "Hall ID already exists.");
        return;
    }

    nasala::Hall newHall(id.toStdString(), name.toStdString());
    
    // Automatically create a default 10x10 room for this new hall so it is immediately usable
    std::string defaultRoomID = id.toStdString() + "_ROOM";
    std::string defaultRoomName = name.toStdString() + " Room";
    nasala::Room defaultRoom(defaultRoomID, defaultRoomName, 10, 10);
    newHall.addRoom(defaultRoom);

    nasala::HallRepository hallRepo(m_db);
    if (hallRepo.save(newHall)) {
        m_manager.addHall(newHall);
        populateHalls(id); // auto-select the new hall
        emit roomChanged();
    } else {
        QMessageBox::critical(this, "Error", "Failed to save Hall to Database.");
    }
}

void RoomManagerPage::onRemoveHall() {
    QString id = selectedHallID();
    if (id.isEmpty()) return;

    auto confirm = QMessageBox::question(this, "Confirm Delete", QString("Are you sure you want to delete Hall %1?").arg(id));
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
    dialog.setWindowTitle("Add Room");
    auto* form = new QFormLayout(&dialog);

    auto* idEdit = new QLineEdit(&dialog);
    auto* nameEdit = new QLineEdit(&dialog);
    auto* rowsSpin = new QSpinBox(&dialog);
    rowsSpin->setRange(1, 100); rowsSpin->setValue(6);
    auto* colsSpin = new QSpinBox(&dialog);
    colsSpin->setRange(1, 100); colsSpin->setValue(6);

    auto* isolatedCheck = new QCheckBox(&dialog);
    auto* accessibleCheck = new QCheckBox(&dialog);

    form->addRow("Room ID (e.g. ROOM_101):", idEdit);
    form->addRow("Room Name:", nameEdit);
    form->addRow("Rows:", rowsSpin);
    form->addRow("Columns:", colsSpin);
    form->addRow("Is Isolated (Classroom):", isolatedCheck);
    form->addRow("Is Accessible:", accessibleCheck);

    auto* bbox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    form->addRow(bbox);
    connect(bbox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(bbox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted) return;

    QString roomID = idEdit->text().trimmed();
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

    nasala::Room newRoom(roomID.toStdString(), roomName.toStdString(), rowsSpin->value(), colsSpin->value(),
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

    auto confirm = QMessageBox::question(this, "Confirm Delete", QString("Are you sure you want to delete Room %1?").arg(roomID));
    if (confirm != QMessageBox::Yes) return;

    nasala::RoomRepository roomRepo(m_db);
    if (roomRepo.remove(roomID.toStdString())) {
        auto* hall = m_manager.findHall(hallID.toStdString());
        if (hall) {
            hall->removeRoom(roomID.toStdString());
        }
        populateRooms(hallID);
        emit roomChanged();
    } else {
        QMessageBox::critical(this, "Error", "Failed to remove Room from Database.");
    }
}

void RoomManagerPage::refresh() {
    loadData();
}
