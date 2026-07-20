#include "../include/seatingpage.h"
#include "../include/subblockdialog.h"
#include "../../nasala_hall_room/include/HallRepository.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QFrame>
#include <QTableWidget>
#include <QHeaderView>
#include <QScrollArea>
#include <QMessageBox>
#include <QFont>

SeatingPage::SeatingPage(HallModel *model, sqlite3* db, QWidget *parent)
    : QWidget(parent), m_model(model), m_db(db)
{
    buildUi();
}

void SeatingPage::buildUi() {
    auto *vl = new QVBoxLayout(this);
    vl->setContentsMargins(28,24,28,24);
    vl->setSpacing(16);

    // Header
    auto *titleLbl = new QLabel("Examination Hall Seating");
    titleLbl->setStyleSheet("font-size:22px; font-weight:bold; color:#003366;");
    auto *subLbl = new QLabel("Interactive hall map - click any sub-block to manage seat assignments");
    subLbl->setStyleSheet("font-size:12px; color:#6b7a8d;");
    vl->addWidget(titleLbl);
    vl->addWidget(subLbl);

    auto *sep = new QFrame; sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("background:#d0d9e8;"); sep->setFixedHeight(1);
    vl->addWidget(sep);

    // Action toolbar
    auto *toolbar = new QFrame;
    toolbar->setStyleSheet("QFrame { background:#f0f4fa; border:1px solid #e0e7ef; border-radius:6px; }");
    auto *tbl = new QHBoxLayout(toolbar);
    tbl->setContentsMargins(12,10,12,10); tbl->setSpacing(10);

    auto *autoBtn = new QPushButton("Auto-Assign All");
    autoBtn->setStyleSheet(
        "QPushButton { background:#003366; color:white; border:none; border-radius:6px; "
        "padding:8px 16px; font-weight:bold; font-size:12px; }"
        "QPushButton:hover { background:#1a4a7a; }");
    auto *clearBtn = new QPushButton("Clear All Assignments");
    clearBtn->setStyleSheet(
        "QPushButton { background:white; color:#8b0000; border:1.5px solid #8b0000; "
        "border-radius:6px; padding:7px 14px; font-size:12px; }"
        "QPushButton:hover { background:#fff0f0; }");

    m_statusLbl = new QLabel("Ready");
    m_statusLbl->setStyleSheet("font-size:11px; color:#6b7a8d; font-style:italic;");

    connect(autoBtn,  &QPushButton::clicked, this, &SeatingPage::onAutoAssign);
    connect(clearBtn, &QPushButton::clicked, this, &SeatingPage::onClearAll);

    auto *roomLbl = new QLabel("Active Room: ");
    roomLbl->setStyleSheet("font-size:12px; font-weight:bold; color:#003366;");
    m_roomSelector = new QComboBox;
    m_roomSelector->setStyleSheet("QComboBox { padding:6px 12px; border:1px solid #d0d9e8; border-radius:6px; font-size:12px; }");
    connect(m_roomSelector, &QComboBox::activated, this, &SeatingPage::onRoomSelected);

    m_toggleInfoBtn = new QPushButton("Info");
    m_toggleInfoBtn->setCheckable(true);
    m_toggleInfoBtn->setChecked(false);
    m_toggleInfoBtn->setStyleSheet(
        "QPushButton { background:white; color:#003366; border:1px solid #d0d9e8; "
        "border-radius:6px; padding:4px 8px; font-weight:bold; font-size:14px; }"
        "QPushButton:hover { background:#f0f4fa; }"
        "QPushButton:checked { background:#003366; color:white; }");
    connect(m_toggleInfoBtn, &QPushButton::toggled, this, &SeatingPage::toggleRoomInfo);

    tbl->addWidget(autoBtn);
    tbl->addWidget(clearBtn);
    tbl->addSpacing(20);
    tbl->addWidget(roomLbl);
    tbl->addWidget(m_roomSelector);
    tbl->addWidget(m_toggleInfoBtn);
    tbl->addStretch();
    tbl->addWidget(m_statusLbl);
    vl->addWidget(toolbar);

    // Collapsible Room Info Card
    m_roomInfoCard = new QWidget;
    m_roomInfoCard->setVisible(false);
    auto *infoLayout = new QVBoxLayout(m_roomInfoCard);
    infoLayout->setContentsMargins(0, 0, 0, 0);
    
    QFrame *infoFrame = new QFrame;
    infoFrame->setStyleSheet("QFrame { background:#f8faff; border:1px solid #d0d9e8; border-radius:6px; }");
    auto *ifl = new QHBoxLayout(infoFrame);
    ifl->setContentsMargins(16, 12, 16, 12);
    ifl->setSpacing(20);
    
    m_roomInfoDetails = new QLabel("Select a room to view details");
    m_roomInfoDetails->setStyleSheet("font-size:12px; color:#1a2332;");
    m_roomInfoDetails->setWordWrap(true);
    ifl->addWidget(m_roomInfoDetails);
    ifl->addStretch();
    
    infoLayout->addWidget(infoFrame);
    vl->addWidget(m_roomInfoCard);

    // Splitter: hall map (left) | search panel (right)
    auto *splitter = new QSplitter(Qt::Horizontal);
    splitter->setHandleWidth(6);
    splitter->setStyleSheet("QSplitter::handle { background:#d0d9e8; }");

    // Hall map in scroll area
    auto *mapFrame = new QFrame;
    mapFrame->setStyleSheet("QFrame { background:white; border:1px solid #e0e7ef; border-radius:8px; }");
    auto *mapVL = new QVBoxLayout(mapFrame);
    mapVL->setContentsMargins(12,12,12,12);

    auto *mapTitle = new QLabel("Hall Map - KU Examination Hall");
    mapTitle->setStyleSheet("font-size:13px; font-weight:bold; color:#003366;");
    mapVL->addWidget(mapTitle);

    m_hallMap = new HallMapWidget(m_model, this);
    connect(m_hallMap, &HallMapWidget::subBlockClicked, this, &SeatingPage::onSubBlockClicked);

    auto *scrollArea = new QScrollArea;
    scrollArea->setWidget(m_hallMap);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    mapVL->addWidget(scrollArea);
    splitter->addWidget(mapFrame);

    // Right panel: search + results
    auto *rightPanel = new QFrame;
    rightPanel->setStyleSheet("QFrame { background:white; border:1px solid #e0e7ef; border-radius:8px; }");
    auto *rpl = new QVBoxLayout(rightPanel);
    rpl->setContentsMargins(16,16,16,16); rpl->setSpacing(10);

    auto *searchTitle = new QLabel("Find Student");
    searchTitle->setStyleSheet("font-size:13px; font-weight:bold; color:#003366;");
    rpl->addWidget(searchTitle);

    m_searchBox = new QLineEdit;
    m_searchBox->setPlaceholderText("Name or roll number...");
    m_searchBox->setStyleSheet(
        "QLineEdit { border:1.5px solid #d0d9e8; border-radius:6px; padding:8px 10px; font-size:12px; }"
        "QLineEdit:focus { border-color:#003366; background:#f8faff; }");
    connect(m_searchBox, &QLineEdit::textChanged, this, &SeatingPage::onSearch);
    rpl->addWidget(m_searchBox);

    m_searchResults = new QTableWidget;
    m_searchResults->setColumnCount(4);
    m_searchResults->setHorizontalHeaderLabels({"Roll","Name","Dept","Seat"});
    m_searchResults->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_searchResults->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_searchResults->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_searchResults->verticalHeader()->hide();
    m_searchResults->setStyleSheet(
        "QTableWidget { border:1px solid #d0d9e8; border-radius:6px; font-size:11px; }"
        "QHeaderView::section { background:#003366; color:white; font-weight:bold; "
        "padding:6px; border:none; border-right:1px solid #1a4a7a; }"
        "QTableWidget::item:selected { background:#dce8f7; color:#003366; }");
    connect(m_searchResults, &QTableWidget::cellClicked,
            this, &SeatingPage::onSearchResultClicked);
    rpl->addWidget(m_searchResults, 1);

    auto *hint = new QLabel("Click a result to jump to seat on the map");
    hint->setStyleSheet("font-size:10px; color:#9ab0c8; font-style:italic;");
    rpl->addWidget(hint);

    rightPanel->setMaximumWidth(340);
    splitter->addWidget(rightPanel);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 1);

    vl->addWidget(splitter, 1);
    setLayout(vl);
}

void SeatingPage::reloadRooms() {
    m_roomSelector->blockSignals(true);
    m_roomSelector->clear();
    if (m_db) {
        nasala::HallRepository hallRepo(m_db);
        auto halls = hallRepo.loadAll();
        for (const auto& hall : halls) {
            for (const auto& room : hall.getRooms()) {
                int capacity = room.getRows() * room.getColumns();
                QString badges;
                if (room.isIsolated())   badges += "[ISOLATED] ";
                if (room.isAccessible()) badges += "[ACCESSIBLE]";
                badges = badges.trimmed();

                QString roomID   = QString::fromStdString(room.getRoomID());
                QString hallID   = QString::fromStdString(hall.getHallID());
                QString hallName = QString::fromStdString(hall.getHallName());
                QString roomName = QString::fromStdString(room.getRoomName());

                // Pass the hall NAME as venueCode so setActiveRoom calls
                // normalizeVenueName("Multi Purpose Hall") -> "MPH",
                // normalizeVenueName("Hall B")            -> "HB", etc.
                // MAIN_ROOM passes empty so getSeatName uses block-grid coding.
                QString venueCode = (roomID == "MAIN_ROOM") ? "" : hallName;

                QString displayName = QString("%1  >  %2  (%3x%4 = %5 seats)%6")
                    .arg(hallName).arg(roomName)
                    .arg(room.getRows()).arg(room.getColumns()).arg(capacity)
                    .arg(badges.isEmpty() ? "" : "  " + badges);

                QVariantList varList;
                varList << hallID << roomID << room.getRows() << room.getColumns()
                        << venueCode << room.isIsolated() << room.isAccessible() << roomName;
                m_roomSelector->addItem(displayName, varList);
            }
        }
    }
    for (int i = 0; i < m_roomSelector->count(); ++i) {
        auto data = m_roomSelector->itemData(i).toList();
        if (data.size() >= 2 && data[1].toString() == m_model->activeRoomID()) {
            m_roomSelector->setCurrentIndex(i);
            break;
        }
    }
    m_roomSelector->blockSignals(false);
}

void SeatingPage::refresh() {
    reloadRooms();

    m_hallMap->refresh();
    int occ   = m_model->occupiedCount();
    int total = m_model->allStudents().size();
    int capacity = m_model->rows() * m_model->cols();
    m_statusLbl->setText(QString("%1 / %2 Students assigned  (%3 / %4 seats)")
                         .arg(occ).arg(total)
                         .arg(occ).arg(capacity));
    updateRoomInfo();
    onSearch();
}

void SeatingPage::onRoomSelected(int index) {
    if (index < 0) return;
    auto data = m_roomSelector->itemData(index).toList();
    if (data.size() >= 4) {
        QString hallID = data[0].toString();
        QString roomID = data[1].toString();
        int rows = data[2].toInt();
        int cols = data[3].toInt();
        QString venueCode = data.size() >= 5 ? data[4].toString() : "";
        m_model->setActiveRoom(hallID, roomID, rows, cols, venueCode);
        updateRoomInfo();
        refresh();
    }
}

void SeatingPage::toggleRoomInfo() {
    m_roomInfoCard->setVisible(m_toggleInfoBtn->isChecked());
}

void SeatingPage::updateRoomInfo() {
    int index = m_roomSelector->currentIndex();
    if (index < 0) {
        m_roomInfoDetails->setText("No room selected");
        return;
    }
    
    auto data = m_roomSelector->itemData(index).toList();
    if (data.size() >= 8) {
        int rows = data[2].toInt();
        int cols = data[3].toInt();
        int capacity = rows * cols;
        QString hallName = data[4].toString();
        bool isIsolated = data[5].toBool();
        bool isAccessible = data[6].toBool();
        QString roomName = data[7].toString();
        
        int occ = m_model->occupiedCount();
        int avail = capacity - occ;
        
        QString statusBadge;
        if (occ == capacity) statusBadge = "<span style='background:#f8d7da; color:#721c24; padding:2px 6px; border-radius:4px;'>FULL</span>";
        else if (occ >= capacity * 0.8) statusBadge = "<span style='background:#fff3cd; color:#856404; padding:2px 6px; border-radius:4px;'>ALMOST FULL</span>";
        else statusBadge = "<span style='background:#d4edda; color:#155724; padding:2px 6px; border-radius:4px;'>AVAILABLE</span>";
        
        QString text = QString(
            "<b>Hall:</b> %1 &nbsp;&nbsp;|&nbsp;&nbsp; <b>Room:</b> %2 &nbsp;&nbsp;|&nbsp;&nbsp; <b>Dimensions:</b> %3 rows x %4 cols<br>"
            "<b>Capacity:</b> %5 &nbsp;&nbsp;|&nbsp;&nbsp; <b>Assigned:</b> %6 &nbsp;&nbsp;|&nbsp;&nbsp; <b>Available:</b> %7 &nbsp;&nbsp;|&nbsp;&nbsp; <b>Status:</b> %8<br>"
            "<b>Flags:</b> %9 %10"
        ).arg(hallName).arg(roomName).arg(rows).arg(cols).arg(capacity).arg(occ).arg(avail).arg(statusBadge)
         .arg(isIsolated ? "<span style='background:#e2e3e5; color:#383d41; padding:2px 4px; border-radius:4px;'>[ISOLATED]</span>" : "")
         .arg(isAccessible ? "<span style='background:#cce5ff; color:#004085; padding:2px 4px; border-radius:4px;'>[ACCESSIBLE]</span>" : "");
         
        m_roomInfoDetails->setText(text);
    }

}

void SeatingPage::onAutoAssign() {
    int before = m_model->occupiedCount();
    m_model->autoAssign();
    int after  = m_model->occupiedCount();
    refresh();
    int capacity = m_model->rows() * m_model->cols();
    m_statusLbl->setText(QString("Auto-assigned %1 Students. Total: %2/%3")
                         .arg(after-before).arg(after).arg(capacity));
    QMessageBox::information(this, "Success", QString("Successfully auto-assigned %1 students.").arg(after-before));
}

void SeatingPage::onClearAll() {
    if (QMessageBox::question(this,"Confirm",
        "Clear ALL seat assignments? This cannot be undone.",
        QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes) {
        m_model->clearAllAssignments();
        refresh();
    }
}

void SeatingPage::onSubBlockClicked(const QString &block, int sub) {
    SubBlockDialog dlg(block, sub, m_model, this);
    dlg.exec();
    refresh();
}

void SeatingPage::onSearch() {
    QString q = m_searchBox->text().trimmed();
    QList<UIStudent*> results = q.isEmpty() ? m_model->allStudents()
                                           : m_model->searchStudents(q);
    m_searchResults->setRowCount(0);
    for (UIStudent *s : results) {
        int row = m_searchResults->rowCount();
        m_searchResults->insertRow(row);
        auto setC = [&](int col, const QString &text) {
            auto *item = new QTableWidgetItem(text);
            item->setTextAlignment(Qt::AlignCenter);
            item->setData(Qt::UserRole, s->id);
            m_searchResults->setItem(row, col, item);
        };
        setC(0, s->rollNumber);
        setC(1, s->name);
        setC(2, s->department.left(12));
        setC(3, s->isAssigned() ? s->seatCode() : "-");
    }
}

void SeatingPage::onSearchResultClicked(int row, int) {
    auto *item = m_searchResults->item(row, 0);
    if (!item) return;
    int sid = item->data(Qt::UserRole).toInt();
    m_hallMap->highlightStudent(sid);
    UIStudent *s = m_model->findById(sid);
    if (s && s->isAssigned()) {
        m_statusLbl->setText(QString("Showing: %1 -> %2").arg(s->name, s->seatCode()));
    }
}

void SeatingPage::jumpToStudent(int studentId) {
    m_hallMap->highlightStudent(studentId);
}




