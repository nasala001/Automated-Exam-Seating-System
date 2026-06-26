#include "../include/subblockdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QHeaderView>
#include <QFont>
#include <QMessageBox>
#include <QFrame>

SubBlockDialog::SubBlockDialog(const QString &block, int sub, HallModel *model, QWidget *parent)
    : QDialog(parent), m_block(block), m_sub(sub), m_model(model)
{
    setWindowTitle(QString("Sub-Block %1%2 — Seat Detail").arg(block).arg(sub));
    setMinimumSize(700, 540);
    buildUi();
    refreshTable();
    populateStudentCombo();
}

void SubBlockDialog::buildUi() {
    auto *mainVL = new QVBoxLayout(this);
    mainVL->setContentsMargins(20,20,20,20);
    mainVL->setSpacing(12);

    // Header
    auto *headerRow = new QHBoxLayout;
    m_titleLbl = new QLabel(QString("Block %1 — Sub-block %2").arg(m_block).arg(m_sub));
    m_titleLbl->setStyleSheet("font-size:18px; font-weight:bold; color:#003366;");
    m_statsLbl = new QLabel();
    m_statsLbl->setStyleSheet("font-size:12px; color:#6b7a8d; "
                              "background:#f0f5ff; border:1px solid #b0c4de; "
                              "border-radius:4px; padding:4px 10px;");
    headerRow->addWidget(m_titleLbl);
    headerRow->addStretch();
    headerRow->addWidget(m_statsLbl);
    mainVL->addLayout(headerRow);

    // Separator
    auto *sep = new QFrame;
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("background:#d0d9e8;");
    sep->setFixedHeight(1);
    mainVL->addWidget(sep);

    // Table
    m_table = new QTableWidget(this);
    m_table->setColumnCount(8);
    QStringList headers = {"Bench","Seat","Roll Number","Student Name",
                           "Department","Semester","Subject","Status"};
    m_table->setHorizontalHeaderLabels(headers);
    m_table->setRowCount(HallConst::BENCHES * HallConst::SEATS_PER_BENCH);
    m_table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setAlternatingRowColors(true);
    m_table->verticalHeader()->hide();
    m_table->setStyleSheet(
        "QTableWidget { border:1px solid #d0d9e8; border-radius:6px; font-size:12px; }"
        "QHeaderView::section { background:#003366; color:white; font-weight:bold; padding:8px; border:none; border-right:1px solid #1a4a7a; }"
        "QTableWidget::item:selected { background:#dce8f7; color:#003366; }"
        "QTableWidget::item:alternate { background:#f8fafc; }"
    );
    connect(m_table, &QTableWidget::cellClicked, this, &SubBlockDialog::onCellClicked);
    mainVL->addWidget(m_table);

    // Action toolbar
    auto *toolRow = new QHBoxLayout;
    toolRow->setSpacing(8);

    m_StudentCombo = new QComboBox;
    m_StudentCombo->setMinimumWidth(220);
    m_StudentCombo->setStyleSheet(
        "QComboBox { border:1.5px solid #d0d9e8; border-radius:6px; padding:6px 10px; font-size:12px; }"
        "QComboBox:focus { border-color:#003366; }"
    );

    m_assignBtn = new QPushButton("Assign to Selected Seat");
    m_assignBtn->setStyleSheet(
        "QPushButton { background:#003366; color:white; border:none; border-radius:6px; "
        "padding:8px 16px; font-weight:bold; font-size:12px; }"
        "QPushButton:hover { background:#1a4a7a; }"
        "QPushButton:disabled { background:#9ab0c8; }"
    );
    m_assignBtn->setEnabled(false);

    m_unassignBtn = new QPushButton("Unassign");
    m_unassignBtn->setStyleSheet(
        "QPushButton { background:white; color:#8b0000; border:1.5px solid #8b0000; "
        "border-radius:6px; padding:7px 14px; font-size:12px; }"
        "QPushButton:hover { background:#fff0f0; }"
        "QPushButton:disabled { color:#9ab0c8; border-color:#9ab0c8; }"
    );
    m_unassignBtn->setEnabled(false);

    m_lockBtn = new QPushButton("Lock/Unlock Seat");
    m_lockBtn->setStyleSheet(
        "QPushButton { background:white; color:#003366; border:1.5px solid #003366; "
        "border-radius:6px; padding:7px 14px; font-size:12px; }"
        "QPushButton:hover { background:#e8f0fb; }"
        "QPushButton:disabled { color:#9ab0c8; border-color:#9ab0c8; }"
    );
    m_lockBtn->setEnabled(false);

    auto *closeBtn = new QPushButton("Close");
    closeBtn->setStyleSheet(
        "QPushButton { background:#f0f4fa; color:#6b7a8d; border:1px solid #d0d9e8; "
        "border-radius:6px; padding:8px 16px; font-size:12px; }"
        "QPushButton:hover { background:#e0e7ef; }"
    );
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);

    toolRow->addWidget(new QLabel("Assign Student:"));
    toolRow->addWidget(m_StudentCombo);
    toolRow->addWidget(m_assignBtn);
    toolRow->addWidget(m_unassignBtn);
    toolRow->addWidget(m_lockBtn);
    toolRow->addStretch();
    toolRow->addWidget(closeBtn);
    mainVL->addLayout(toolRow);

    connect(m_assignBtn,   &QPushButton::clicked, this, &SubBlockDialog::onAssignClicked);
    connect(m_unassignBtn, &QPushButton::clicked, this, &SubBlockDialog::onUnassignClicked);
    connect(m_lockBtn,     &QPushButton::clicked, this, &SubBlockDialog::onLockToggle);

    setLayout(mainVL);
}

QColor SubBlockDialog::statusColor(SeatStatus s) const {
    switch(s) {
        case SeatStatus::Occupied: return QColor("#d4edda");
        case SeatStatus::Assigned: return QColor("#cce5ff");
        case SeatStatus::Reserved: return QColor("#fff3cd");
        case SeatStatus::Conflict: return QColor("#f8d7da");
        default:                   return QColor("#f8fafc");
    }
}
QString SubBlockDialog::statusText(SeatStatus s) const {
    switch(s) {
        case SeatStatus::Occupied: return "Occupied";
        case SeatStatus::Assigned: return "Assigned";
        case SeatStatus::Reserved: return "Reserved";
        case SeatStatus::Conflict: return "Conflict";
        default:                   return "Empty";
    }
}

void SubBlockDialog::refreshTable() {
    int occ = m_model->occupiedInSubBlock(m_block, m_sub);
    m_statsLbl->setText(QString("%1 / %2 occupied  (%3%)")
                        .arg(occ).arg(HallConst::PER_SUB)
                        .arg((int)((double)occ/HallConst::PER_SUB*100)));

    int row = 0;
    for (int bench=1; bench<=HallConst::BENCHES; bench++) {
        for (int seat=1; seat<=HallConst::SEATS_PER_BENCH; seat++) {
            const SeatCell &cell = m_model->seatAt(m_block, m_sub, bench, seat);
            UIStudent *st = (cell.studentId != -1) ? m_model->findById(cell.studentId) : nullptr;

            auto setCell = [&](int col, const QString &text) {
                auto *item = new QTableWidgetItem(text);
                item->setTextAlignment(Qt::AlignCenter);
                QColor bg = statusColor(cell.status);
                if (cell.locked) bg = QColor("#e9ecef");
                item->setBackground(bg);
                m_table->setItem(row, col, item);
            };

            setCell(0, QString("Bench %1").arg(bench));
            setCell(1, (seat==1) ? "Left" : "Right");
            setCell(2, st ? st->rollNumber : (cell.locked ? "🔒 Locked" : "—"));
            setCell(3, st ? st->name : "—");
            setCell(4, st ? st->department : "—");
            setCell(5, st ? QString("Sem %1").arg(st->semester) : "—");
            setCell(6, st ? st->subject : "—");

            auto *statusItem = new QTableWidgetItem(statusText(cell.status));
            statusItem->setTextAlignment(Qt::AlignCenter);
            QColor sc = statusColor(cell.status);
            statusItem->setBackground(sc);
            statusItem->setForeground(cell.status==SeatStatus::Conflict ? QColor("#721c24")
                                    : cell.status==SeatStatus::Occupied  ? QColor("#155724")
                                    : QColor("#1a2332"));
            QFont f; f.setBold(cell.status != SeatStatus::Empty);
            statusItem->setFont(f);
            m_table->setItem(row, 7, statusItem);
            row++;
        }
    }
    m_table->resizeColumnsToContents();
    m_table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);
}

void SubBlockDialog::populateStudentCombo() {
    m_StudentCombo->clear();
    m_StudentCombo->addItem("-- Select Unassigned Student --", -1);
    QList<UIStudent*> unassigned = m_model->unassignedStudents();
    for (UIStudent *s : unassigned) {
        m_StudentCombo->addItem(
            QString("%1 — %2 (%3)").arg(s->rollNumber, s->name, s->department),
            s->id
        );
    }
}

void SubBlockDialog::onCellClicked(int row, int) {
    int bench = (row / 2) + 1;
    int seat  = (row % 2) + 1;
    m_selectedBench = bench;
    m_selectedSeat  = seat;

    const SeatCell &cell = m_model->seatAt(m_block, m_sub, bench, seat);
    bool hasStudent = (cell.studentId != -1);

    m_assignBtn->setEnabled(!hasStudent && !cell.locked);
    m_unassignBtn->setEnabled(hasStudent && !cell.locked);
    m_lockBtn->setEnabled(!hasStudent || cell.locked);
    m_lockBtn->setText(cell.locked ? "Unlock Seat" : "Lock Seat");
}

void SubBlockDialog::onAssignClicked() {
    if (m_selectedBench < 1) return;
    int sid = m_StudentCombo->currentData().toInt();
    if (sid < 0) { QMessageBox::warning(this,"No Student","Please select a student."); return; }
    if (!m_model->assignSeat(sid, m_block, m_sub, m_selectedBench, m_selectedSeat)) {
        QMessageBox::warning(this,"Failed","Could not assign seat — it may be locked or occupied.");
        return;
    }
    refreshTable();
    populateStudentCombo();
    m_assignBtn->setEnabled(false);
}

void SubBlockDialog::onUnassignClicked() {
    if (m_selectedBench < 1) return;
    m_model->unassignSeat(m_block, m_sub, m_selectedBench, m_selectedSeat);
    refreshTable();
    populateStudentCombo();
    m_unassignBtn->setEnabled(false);
    m_assignBtn->setEnabled(false);
    m_lockBtn->setEnabled(false);
}

void SubBlockDialog::onLockToggle() {
    if (m_selectedBench < 1) return;
    const SeatCell &cell = m_model->seatAt(m_block, m_sub, m_selectedBench, m_selectedSeat);
    m_model->lockSeat(m_block, m_sub, m_selectedBench, m_selectedSeat, !cell.locked);
    refreshTable();
    // recheck selection
    onCellClicked((m_selectedBench-1)*2 + (m_selectedSeat-1), 0);
}
