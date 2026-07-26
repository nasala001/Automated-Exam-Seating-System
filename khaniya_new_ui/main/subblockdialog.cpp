#include "../include/subblockdialog.h"
#include <QApplication>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QSizePolicy>
#include <QVBoxLayout>

// ════════════════════════════════════════════════════════════════════════════
// SeatCard Implementation
// ════════════════════════════════════════════════════════════════════════════

SeatCard::SeatCard(const QString &seatCode, int bench, int seat, QWidget *parent)
    : QFrame(parent), m_seatCode(seatCode), m_bench(bench), m_seat(seat)
{
    setFixedSize(170, 92);
    setCursor(Qt::PointingHandCursor);
    setFrameStyle(QFrame::NoFrame);

    auto *vl = new QVBoxLayout(this);
    vl->setContentsMargins(10, 8, 10, 8);
    vl->setSpacing(2);

    // Seat Code header  (e.g. A1-B2-S1)
    m_codeLabel = new QLabel(seatCode, this);
    m_codeLabel->setStyleSheet("font-size:9px; font-weight:600; color:rgba(255,255,255,0.70); background:transparent;");
    m_codeLabel->setAlignment(Qt::AlignLeft);

    // Roll number  (dominant)
    m_rollLabel = new QLabel("—", this);
    m_rollLabel->setStyleSheet("font-size:20px; font-weight:bold; color:white; background:transparent;");
    m_rollLabel->setAlignment(Qt::AlignCenter);

    // Student name
    m_nameLabel = new QLabel("Empty Seat", this);
    m_nameLabel->setStyleSheet("font-size:10px; color:rgba(255,255,255,0.85); background:transparent;");
    m_nameLabel->setAlignment(Qt::AlignCenter);
    m_nameLabel->setWordWrap(false);

    // Status pill
    m_statusLabel = new QLabel("Empty", this);
    m_statusLabel->setStyleSheet("font-size:9px; font-weight:600; color:rgba(255,255,255,0.65); background:transparent;");
    m_statusLabel->setAlignment(Qt::AlignRight);

    vl->addWidget(m_codeLabel);
    vl->addWidget(m_rollLabel, 1);
    vl->addWidget(m_nameLabel);
    vl->addWidget(m_statusLabel);
}

// Forward declaration needed: statusText is defined later in this file
static QString seatCardStatusText(SeatStatus s) {
    switch(s) {
        case SeatStatus::Occupied: return "Occupied";
        case SeatStatus::Assigned: return "Assigned";
        case SeatStatus::Reserved: return "Reserved";
        case SeatStatus::Conflict: return "Conflict";
        default:                   return "Empty";
    }
}

void SeatCard::update(const SeatCell &cell, UIStudent *st, bool selected) {
    m_selected = selected;

    if (cell.locked) {
        m_rollLabel->setText("");
        m_nameLabel->setText("Locked");
        m_statusLabel->setText("LOCKED");
    } else if (st) {
        QString roll = st->rollNumber.isEmpty() ? QString::number(st->id) : st->rollNumber;
        m_rollLabel->setText(roll);
        QString name = st->name;
        if (name.length() > 20) name = name.left(18) + "…";
        m_nameLabel->setText(name);
        if (cell.status == SeatStatus::Conflict || st->status == SeatStatus::Conflict) {
            m_statusLabel->setText("⚠️ CONFLICT");
            setToolTip(QString("CONFLICT DETECTED\nStudent: %1 (%2)\n⚠️ Seating Rule Conflict").arg(st->name, roll));
        } else {
            m_statusLabel->setText(seatCardStatusText(cell.status).toUpper());
            setToolTip(QString("Seat %1: %2 (%3)").arg(m_seat).arg(st->name).arg(roll));
        }
    } else {
        m_rollLabel->setText("—");
        m_nameLabel->setText("Empty Seat");
        m_statusLabel->setText("AVAILABLE");
    }

    applyStyle(cell);
}

void SeatCard::applyStyle(const SeatCell &cell) {
    QString bg, border;
    int radius = 10;

    if (cell.locked) {
        bg = "#64748b";
        border = m_selected ? "3px solid #FFD700" : "2px solid #475569";
    } else if (cell.studentId != -1) {
        switch (cell.status) {
            case SeatStatus::Occupied: bg = "#16a34a"; border = m_selected ? "3px solid #FFD700" : "2px solid #15803d"; break;
            case SeatStatus::Conflict: bg = "#dc2626"; border = m_selected ? "3px solid #FFD700" : "2px solid #b91c1c"; break;
            case SeatStatus::Reserved: bg = "#d97706"; border = m_selected ? "3px solid #FFD700" : "2px solid #b45309"; break;
            default:                   bg = "#2563eb"; border = m_selected ? "3px solid #FFD700" : "2px solid #1d4ed8"; break;
        }
    } else {
        bg = m_hovered ? "#475569" : "#334155";
        border = m_selected ? "3px solid #FFD700" : "1.5px solid #64748b";
    }

    // Drop shadow via outer container styling
    setStyleSheet(QString(
        "SeatCard { background-color: %1; border: %2; border-radius: %3px; }"
    ).arg(bg).arg(border).arg(radius));
}

QString SeatCard::statusText(SeatStatus s) const { return seatCardStatusText(s); }


void SeatCard::mousePressEvent(QMouseEvent *) {
    emit clicked(m_bench, m_seat);
}

void SeatCard::enterEvent(QEnterEvent *) {
    m_hovered = true;
    repaint();
}

void SeatCard::leaveEvent(QEvent *) {
    m_hovered = false;
    repaint();
}

void SeatCard::paintEvent(QPaintEvent *e) {
    QFrame::paintEvent(e);
}

// ════════════════════════════════════════════════════════════════════════════
// SubBlockDialog Implementation
// ════════════════════════════════════════════════════════════════════════════

SubBlockDialog::SubBlockDialog(const QString &block, int sub, HallModel *model, QWidget *parent)
    : QDialog(parent), m_block(block), m_sub(sub), m_model(model)
    , m_numBenches(HallConst::BENCHES), m_seatsPerBench(HallConst::SEATS_PER_BENCH)
{
    setWindowTitle(QString("Block %1 · Sub-block %2 — Seating View").arg(block).arg(sub));
    setMinimumSize(1060, 640);
    buildUi();
    refresh();
    populateStudentCombo();
}

// ─────────────────────────────────────────────────────────────────────────────
// Build UI
// ─────────────────────────────────────────────────────────────────────────────

void SubBlockDialog::buildUi() {
    setStyleSheet("QDialog { background: #f1f5f9; }");

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(20, 20, 20, 20);
    root->setSpacing(14);

    // ── Header ──
    {
        auto *hdr = new QHBoxLayout;

        auto *icon = new QLabel("", this);
        icon->setStyleSheet("font-size:28px;");

        auto *titlesVL = new QVBoxLayout;
        m_titleLbl = new QLabel(QString("Block <b>%1</b> · Sub-block <b>%2</b>").arg(m_block).arg(m_sub));
        m_titleLbl->setStyleSheet("font-size:20px; color:#0f172a;");
        m_statsLbl = new QLabel();
        m_statsLbl->setStyleSheet("font-size:12px; color:#64748b;");
        titlesVL->addWidget(m_titleLbl);
        titlesVL->addWidget(m_statsLbl);
        titlesVL->setSpacing(2);

        // Legend
        auto legendMake = [&](const QString &color, const QString &label) {
            auto *w = new QWidget(this);
            auto *hl = new QHBoxLayout(w);
            hl->setContentsMargins(0,0,0,0); hl->setSpacing(5);
            auto *dot = new QLabel(w);
            dot->setFixedSize(14,14);
            dot->setStyleSheet(QString("background:%1; border-radius:7px;").arg(color));
            hl->addWidget(dot);
            hl->addWidget(new QLabel(label, w));
            return w;
        };
        auto *legendHL = new QHBoxLayout;
        legendHL->setSpacing(14);
        legendHL->addWidget(legendMake("#334155", "Empty"));
        legendHL->addWidget(legendMake("#16a34a", "Occupied"));
        legendHL->addWidget(legendMake("#2563eb", "Assigned"));
        legendHL->addWidget(legendMake("#dc2626", "Conflict"));
        legendHL->addWidget(legendMake("#64748b", "Locked"));

        hdr->addWidget(icon);
        hdr->addLayout(titlesVL);
        hdr->addStretch();
        hdr->addLayout(legendHL);
        root->addLayout(hdr);
    }

    // ── Separator ──
    auto *sep = new QFrame(this);
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("background:#cbd5e1;");
    sep->setFixedHeight(1);
    root->addWidget(sep);

    // ── Tab Widget ──
    m_tabs = new QTabWidget(this);
    m_tabs->setStyleSheet(
        "QTabWidget::pane { border:1px solid #e2e8f0; border-radius:8px; background:white; }"
        "QTabBar::tab { background:#e2e8f0; border:none; border-top-left-radius:6px; "
        "border-top-right-radius:6px; padding:10px 24px; font-size:13px; font-weight:600; "
        "color:#64748b; margin-right:3px; }"
        "QTabBar::tab:selected { background:white; color:#003366; border-bottom:3px solid #003366; }"
        "QTabBar::tab:hover { background:#f1f5f9; }"
    );

    buildVisualTab();
    buildListTab();

    m_tabs->addTab(m_visualTab,   "  Visual Seat Layout  ");
    m_tabs->addTab(m_listTab,     "  Students Details List  ");
    root->addWidget(m_tabs, 1);
    setLayout(root);
}

// ─────────────────────────────────────────────────────────────────────────────
// Tab 1 — Visual Grid Tab
// ─────────────────────────────────────────────────────────────────────────────

void SubBlockDialog::buildVisualTab() {
    m_visualTab = new QWidget;
    auto *outerHL = new QHBoxLayout(m_visualTab);
    outerHL->setContentsMargins(16, 16, 16, 16);
    outerHL->setSpacing(20);

    // ── LEFT: Grid ──
    auto *gridOuter = new QWidget;
    auto *gridOuterVL = new QVBoxLayout(gridOuter);
    gridOuterVL->setContentsMargins(0,0,0,0);
    gridOuterVL->setSpacing(8);

    auto *gridLabel = new QLabel("  Seat Grid — Front of Room", gridOuter);
    gridLabel->setStyleSheet("font-size:12px; font-weight:600; color:#475569; margin-bottom:4px;");
    gridOuterVL->addWidget(gridLabel);

    // Scrollable grid area
    m_gridScroll = new QScrollArea(gridOuter);
    m_gridScroll->setWidgetResizable(true);
    m_gridScroll->setFrameShape(QFrame::NoFrame);
    m_gridScroll->setStyleSheet("QScrollArea { background:transparent; }");

    auto *gridContainer = new QWidget;
    gridContainer->setStyleSheet("background:transparent;");
    auto *gridLayout = new QGridLayout(gridContainer);
    gridLayout->setSpacing(12);
    gridLayout->setContentsMargins(8, 8, 8, 8);

    // Column headers ("Seat 1", "Seat 2", …) — use same aisle-aware mapping as cards
    for (int s = 0; s < m_seatsPerBench; ++s) {
        int gridCol = (m_seatsPerBench == 2 && s == 1) ? 3 : (s + 1);
        auto *colHdr = new QLabel(QString("Seat %1").arg(s + 1), gridContainer);
        colHdr->setAlignment(Qt::AlignCenter);
        colHdr->setStyleSheet("font-size:11px; font-weight:700; color:#94a3b8; letter-spacing:1px;");
        colHdr->setFixedWidth(170);
        gridLayout->addWidget(colHdr, 0, gridCol);
    }

    // Aisle separator column (between seats 1 and 2 when seatsPerBench > 1)
    int aisleCol = -1;
    if (m_seatsPerBench == 2) {
        aisleCol = 2; // after seat 1 (column index 1), aisle at index 2, seat 2 at 3
        auto *aisleHdr = new QLabel("  ", gridContainer);
        aisleHdr->setFixedWidth(24);
        gridLayout->addWidget(aisleHdr, 0, 2);
    }

    // Seat card rows
    m_seatCards.clear();
    m_seatCards.resize(m_numBenches);

    for (int b = 0; b < m_numBenches; ++b) {
        m_seatCards[b].resize(m_seatsPerBench);

        // Row label "Bench 1", "Bench 2", …
        auto *rowLbl = new QLabel(QString("Bench %1").arg(b + 1), gridContainer);
        rowLbl->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        rowLbl->setStyleSheet("font-size:11px; font-weight:700; color:#94a3b8; letter-spacing:1px; min-width:56px;");
        gridLayout->addWidget(rowLbl, b + 1, 0);

        for (int s = 0; s < m_seatsPerBench; ++s) {
            int bi = m_model->blockIndex(m_block);
            int r  = (bi / 3) * 6 + b;
            int c  = (bi % 3) * 6 + (m_sub - 1) * 2 + s;
            QString code = m_model->getSeatName(r, c);

            auto *card = new SeatCard(code, b + 1, s + 1, gridContainer);
            m_seatCards[b][s] = card;

            // Aisle-aware column placement
            int gridCol = (m_seatsPerBench == 2 && s == 1) ? 3 : (s + 1);
            gridLayout->addWidget(card, b + 1, gridCol);

            connect(card, &SeatCard::clicked, this, &SubBlockDialog::onSeatCardClicked);
        }

        // Aisle divider widget (only once per row)
        if (m_seatsPerBench == 2) {
            auto *aisle = new QWidget(gridContainer);
            aisle->setFixedWidth(24);
            aisle->setStyleSheet("background: transparent;");
            gridLayout->addWidget(aisle, b + 1, 2);
        }
    }

    // "FRONT" label
    auto *frontLbl = new QLabel("▲  FRONT OF ROOM / INVIGILATOR DESK  ▲", gridContainer);
    frontLbl->setAlignment(Qt::AlignCenter);
    frontLbl->setStyleSheet("font-size:10px; font-weight:600; color:#94a3b8; letter-spacing:2px; padding:6px;");
    gridLayout->addWidget(frontLbl, m_numBenches + 1, 0, 1, m_seatsPerBench + 2);

    m_gridScroll->setWidget(gridContainer);
    gridOuterVL->addWidget(m_gridScroll, 1);

    outerHL->addWidget(gridOuter, 3);

    // ── RIGHT: Profile Card + Action Panel ──
    auto *rightVL = new QVBoxLayout;
    rightVL->setSpacing(8);

    // Profile Card
    m_profileCard = new QFrame;
    m_profileCard->setStyleSheet(
        "QFrame#profileCard { background: white; border: 1px solid #e2e8f0; border-radius: 10px; }"
    );
    m_profileCard->setObjectName("profileCard");
    auto *profileVL = new QVBoxLayout(m_profileCard);
    profileVL->setContentsMargins(12, 10, 12, 10);
    profileVL->setSpacing(4);

    auto *profileHdr = new QLabel("Student Profile", m_profileCard);
    profileHdr->setStyleSheet("font-size:13px; font-weight:700; color:#003366; background:transparent; border:none;");
    profileVL->addWidget(profileHdr);

    auto *profileSep = new QFrame(m_profileCard);
    profileSep->setFrameShape(QFrame::HLine);
    profileSep->setStyleSheet("background:#e2e8f0;");
    profileSep->setFixedHeight(1);
    profileVL->addWidget(profileSep);

    // Status badge — uses objectName to avoid parent override
    m_profileStatus = new QLabel("—", m_profileCard);
    m_profileStatus->setObjectName("profileStatusBadge");
    m_profileStatus->setAlignment(Qt::AlignCenter);
    m_profileStatus->setWordWrap(true);
    m_profileStatus->setMinimumHeight(28);
    m_profileStatus->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_profileStatus->setStyleSheet(
        "QLabel#profileStatusBadge { font-size:12px; font-weight:700; background:#f1f5f9; border:1.5px solid #cbd5e1; "
        "border-radius:6px; padding:5px 8px; color:#64748b; }"
    );
    profileVL->addWidget(m_profileStatus);

    // Disability accessibility badge
    m_profileDisabilityTag = new QLabel(m_profileCard);
    m_profileDisabilityTag->setObjectName("profileDisabilityBadge");
    m_profileDisabilityTag->setAlignment(Qt::AlignCenter);
    m_profileDisabilityTag->setWordWrap(true);
    m_profileDisabilityTag->setMinimumHeight(24);
    m_profileDisabilityTag->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_profileDisabilityTag->setVisible(false);
    profileVL->addWidget(m_profileDisabilityTag);

    m_profileEmptyMsg = new QLabel("Click a seat to view\nstudent details.", m_profileCard);
    m_profileEmptyMsg->setAlignment(Qt::AlignCenter);
    m_profileEmptyMsg->setStyleSheet("color:#94a3b8; font-size:12px; padding:14px; background:transparent; border:none;");
    profileVL->addWidget(m_profileEmptyMsg);

    // Conflict detail callout
    m_profileConflictDetail = new QLabel(m_profileCard);
    m_profileConflictDetail->setObjectName("profileConflictBadge");
    m_profileConflictDetail->setWordWrap(true);
    m_profileConflictDetail->setMinimumHeight(24);
    m_profileConflictDetail->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_profileConflictDetail->setVisible(false);
    profileVL->addWidget(m_profileConflictDetail);

    auto makeRow = [&](const QString &label, QLabel *&out) {
        auto *rowFrame = new QWidget(m_profileCard);
        auto *rowHL = new QHBoxLayout(rowFrame);
        rowHL->setContentsMargins(0, 1, 0, 1);
        rowHL->setSpacing(6);
        auto *keyLbl = new QLabel(label + ":", rowFrame);
        keyLbl->setStyleSheet("font-size:11px; font-weight:700; color:#64748b; min-width:70px; background:transparent; border:none;");
        out = new QLabel("—", rowFrame);
        out->setStyleSheet("font-size:11px; color:#1e293b; background:transparent; border:none;");
        out->setWordWrap(true);
        rowHL->addWidget(keyLbl);
        rowHL->addWidget(out, 1);
        profileVL->addWidget(rowFrame);
        rowFrame->setVisible(false);
        return rowFrame;
    };

    QList<QWidget*> detailRows;
    detailRows << makeRow("Seat Code",   m_profileSeatCode);
    detailRows << makeRow("Roll No.",    m_profileRoll);
    detailRows << makeRow("Name",        m_profileName);
    detailRows << makeRow("Department",  m_profileDept);
    detailRows << makeRow("Semester",    m_profileSem);
    detailRows << makeRow("Section",     m_profileSec);
    detailRows << makeRow("Subject",     m_profileSub);
    Q_UNUSED(detailRows)

    profileVL->addStretch(1);

    // Wrap in scroll area so content never clips
    auto *profileScroll = new QScrollArea;
    profileScroll->setWidget(m_profileCard);
    profileScroll->setWidgetResizable(true);
    profileScroll->setFrameShape(QFrame::NoFrame);
    profileScroll->setStyleSheet("QScrollArea { background: transparent; border: none; }");
    rightVL->addWidget(profileScroll, 2);

    // Action Panel
    auto *actionCard = new QFrame;
    actionCard->setStyleSheet(
        "QFrame { background: white; border: 1px solid #e2e8f0; border-radius: 10px; }"
    );
    auto *actionVL = new QVBoxLayout(actionCard);
    actionVL->setContentsMargins(14, 14, 14, 14);
    actionVL->setSpacing(8);

    auto *actionHdr = new QLabel("Actions", actionCard);
    actionHdr->setStyleSheet("font-size:14px; font-weight:700; color:#003366;");
    actionVL->addWidget(actionHdr);

    m_studentCombo = new QComboBox(actionCard);
    m_studentCombo->setStyleSheet(
        "QComboBox { border:1.5px solid #cbd5e1; border-radius:6px; padding:7px 10px; "
        "font-size:12px; background:white; }"
        "QComboBox:focus { border-color:#003366; }"
        "QComboBox::drop-down { border:none; }"
    );
    actionVL->addWidget(m_studentCombo);

    m_assignBtn = new QPushButton("Assign to Selected Seat", actionCard);
    m_assignBtn->setStyleSheet(
        "QPushButton { background:#003366; color:white; border:none; border-radius:7px; "
        "padding:9px; font-weight:700; font-size:12px; }"
        "QPushButton:hover { background:#1e4f8f; }"
        "QPushButton:disabled { background:#cbd5e1; color:#94a3b8; }"
    );
    m_assignBtn->setEnabled(false);
    actionVL->addWidget(m_assignBtn);

    auto *rowHL2 = new QHBoxLayout;
    m_unassignBtn = new QPushButton("Unassign", actionCard);
    m_unassignBtn->setStyleSheet(
        "QPushButton { background:white; color:#dc2626; border:1.5px solid #dc2626; "
        "border-radius:7px; padding:8px; font-size:12px; font-weight:600; }"
        "QPushButton:hover { background:#fef2f2; }"
        "QPushButton:disabled { color:#cbd5e1; border-color:#cbd5e1; }"
    );
    m_unassignBtn->setEnabled(false);
    rowHL2->addWidget(m_unassignBtn);

    m_lockBtn = new QPushButton("Lock Seat", actionCard);
    m_lockBtn->setStyleSheet(
        "QPushButton { background:white; color:#475569; border:1.5px solid #475569; "
        "border-radius:7px; padding:8px; font-size:12px; font-weight:600; }"
        "QPushButton:hover { background:#f8fafc; }"
        "QPushButton:disabled { color:#cbd5e1; border-color:#cbd5e1; }"
    );
    m_lockBtn->setEnabled(false);
    rowHL2->addWidget(m_lockBtn);
    actionVL->addLayout(rowHL2);

    auto *closeBtn = new QPushButton("Close", actionCard);
    closeBtn->setStyleSheet(
        "QPushButton { background:#f1f5f9; color:#64748b; border:1px solid #e2e8f0; "
        "border-radius:7px; padding:8px; font-size:12px; }"
        "QPushButton:hover { background:#e2e8f0; }"
    );
    actionVL->addWidget(closeBtn);

    connect(m_assignBtn,   &QPushButton::clicked, this, &SubBlockDialog::onAssignClicked);
    connect(m_unassignBtn, &QPushButton::clicked, this, &SubBlockDialog::onUnassignClicked);
    connect(m_lockBtn,     &QPushButton::clicked, this, &SubBlockDialog::onLockToggle);
    connect(closeBtn,      &QPushButton::clicked, this, &QDialog::accept);

    rightVL->addWidget(actionCard, 1);
    auto *rightWidget = new QWidget;
    rightWidget->setMinimumWidth(320);
    rightWidget->setLayout(rightVL);
    outerHL->addWidget(rightWidget, 1);
}

// ─────────────────────────────────────────────────────────────────────────────
// Tab 2 — Tabular Detail List
// ─────────────────────────────────────────────────────────────────────────────

void SubBlockDialog::buildListTab() {
    m_listTab = new QWidget;
    auto *vl = new QVBoxLayout(m_listTab);
    vl->setContentsMargins(14, 14, 14, 14);

    m_table = new QTableWidget(m_numBenches * m_seatsPerBench, 8, m_listTab);
    m_table->setHorizontalHeaderLabels({"Seat Code","Roll No.","Student Name",
                                        "Department","Semester","Section","Subject","Status"});
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setAlternatingRowColors(true);
    m_table->verticalHeader()->hide();
    m_table->setShowGrid(false);
    m_table->setStyleSheet(
        "QTableWidget { border:none; font-size:12px; outline:0; }"
        "QTableWidget::item { padding:8px 10px; border-bottom:1px solid #f1f5f9; }"
        "QTableWidget::item:selected { background:#dbeafe; color:#1e40af; }"
        "QTableWidget::item:alternate { background:#f8fafc; }"
        "QHeaderView::section { background:#0f172a; color:white; font-weight:700; font-size:12px; "
        "padding:10px; border:none; border-right:1px solid #334155; }"
    );
    connect(m_table, &QTableWidget::cellClicked, this, &SubBlockDialog::onCellClicked);
    vl->addWidget(m_table);
}

// ─────────────────────────────────────────────────────────────────────────────
// Refresh
// ─────────────────────────────────────────────────────────────────────────────

void SubBlockDialog::refresh() {
    refreshGrid();
    refreshTable();
    if (m_selectedBench >= 1)
        updateProfilePanel(m_selectedBench, m_selectedSeat);
}

void SubBlockDialog::refreshGrid() {
    int occ = m_model->occupiedInSubBlock(m_block, m_sub);
    int cap = m_numBenches * m_seatsPerBench;
    m_statsLbl->setText(QString("%1 / %2 seats occupied  ·  %3% utilization")
                        .arg(occ).arg(cap).arg(cap > 0 ? (int)(100.0*occ/cap) : 0));

    for (int b = 0; b < m_numBenches; ++b) {
        for (int s = 0; s < m_seatsPerBench; ++s) {
            const SeatCell &cell = m_model->seatAt(m_block, m_sub, b + 1, s + 1);
            UIStudent *st = (cell.studentId != -1) ? m_model->findById(cell.studentId) : nullptr;
            bool sel = (m_selectedBench == b + 1 && m_selectedSeat == s + 1);
            m_seatCards[b][s]->update(cell, st, sel);
        }
    }
}

void SubBlockDialog::refreshTable() {
    int row = 0;
    for (int b = 1; b <= m_numBenches; ++b) {
        for (int s = 1; s <= m_seatsPerBench; ++s) {
            const SeatCell &cell = m_model->seatAt(m_block, m_sub, b, s);
            UIStudent *st = (cell.studentId != -1) ? m_model->findById(cell.studentId) : nullptr;

            int bi = m_model->blockIndex(m_block);
            int r  = (bi / 3) * 6 + (b - 1);
            int c  = (bi % 3) * 6 + (m_sub - 1) * 2 + (s - 1);
            QString code = m_model->getSeatName(r, c);

            QColor bg = statusBgColor(cell.status);
            if (cell.locked) bg = QColor("#f1f5f9");

            auto setC = [&](int col, const QString &text, Qt::Alignment align = Qt::AlignVCenter | Qt::AlignLeft) {
                auto *item = new QTableWidgetItem(text);
                item->setTextAlignment(align);
                item->setBackground(bg);
                m_table->setItem(row, col, item);
            };

            setC(0, code, Qt::AlignCenter);
            setC(1, st ? st->rollNumber : (cell.locked ? "" : "—"), Qt::AlignCenter);
            setC(2, st ? st->name : "—");
            setC(3, st ? st->department : "—");
            setC(4, st ? QString("Sem %1").arg(st->semester) : "—", Qt::AlignCenter);
            setC(5, st ? (st->section.isEmpty() ? "—" : st->section) : "—", Qt::AlignCenter);
            setC(6, st ? st->subject : "—");

            QString stxt = cell.locked ? "Locked" : statusText(cell.status);
            auto *statusItem = new QTableWidgetItem(stxt);
            statusItem->setTextAlignment(Qt::AlignCenter);
            statusItem->setBackground(bg);
            QColor fg = cell.status==SeatStatus::Conflict ? QColor("#b91c1c")
                      : cell.status==SeatStatus::Occupied  ? QColor("#15803d")
                      : QColor("#64748b");
            statusItem->setForeground(fg);
            QFont f; f.setBold(true); statusItem->setFont(f);
            m_table->setItem(row, 7, statusItem);
            row++;
        }
    }
    m_table->resizeColumnsToContents();
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
}

// ─────────────────────────────────────────────────────────────────────────────
// Profile Panel
// ─────────────────────────────────────────────────────────────────────────────

void SubBlockDialog::updateProfilePanel(int bench, int seat) {
    const SeatCell &cell = m_model->seatAt(m_block, m_sub, bench, seat);
    UIStudent *st = (cell.studentId != -1) ? m_model->findById(cell.studentId) : nullptr;

    int bi = m_model->blockIndex(m_block);
    int r  = (bi / 3) * 6 + (bench - 1);
    int c  = (bi % 3) * 6 + (m_sub - 1) * 2 + (seat - 1);
    QString code = m_model->getSeatName(r, c);

    m_profileSeatCode->setText(code);
    m_profileEmptyMsg->setVisible(false);

    // Show parent row widgets (they are siblings inside profileVL)
    // We locate them via property set during makeRow
    auto showParent = [](QLabel *lbl) {
        if (lbl && lbl->parentWidget()) lbl->parentWidget()->setVisible(true);
    };
    showParent(m_profileSeatCode);
    showParent(m_profileRoll);
    showParent(m_profileName);
    showParent(m_profileDept);
    showParent(m_profileSem);
    showParent(m_profileSec);
    showParent(m_profileSub);

    if (cell.locked) {
        m_profileStatus->setText("🔒 SEAT LOCKED");
        m_profileStatus->setStyleSheet(
            "QLabel#profileStatusBadge { font-size:12px; font-weight:700; background:#f1f5f9; "
            "border:1.5px solid #94a3b8; border-radius:6px; padding:5px 8px; color:#475569; }");
        m_profileDisabilityTag->setVisible(false);
        m_profileConflictDetail->setVisible(false);
        m_profileRoll->setText("—");
        m_profileName->setText("—");
        m_profileDept->setText("—");
        m_profileSem->setText("—");
        m_profileSec->setText("—");
        m_profileSub->setText("—");
    } else if (st) {
        if (cell.status == SeatStatus::Conflict || st->status == SeatStatus::Conflict) {
            m_profileStatus->setText("⚠️ CONFLICT");
            m_profileStatus->setStyleSheet(
                "QLabel#profileStatusBadge { font-size:12px; font-weight:800; background:#dc2626; "
                "border:2px solid #991b1b; border-radius:6px; padding:5px 8px; color:white; }");
            QString violationDetail = m_model->getViolationDetailForSeat(r, c);
            m_profileConflictDetail->setText("⚠️ " + violationDetail);
            m_profileConflictDetail->setStyleSheet(
                "QLabel#profileConflictBadge { font-size:10px; font-weight:600; color:#991b1b; background:#fee2e2; "
                "border:1.5px solid #fca5a5; border-radius:6px; padding:5px 8px; }");
            m_profileConflictDetail->setVisible(true);
        } else {
            QString statusStr = statusText(cell.status);
            QString statusColor = cell.status == SeatStatus::Occupied ? "#16a34a" : "#2563eb";
            m_profileStatus->setText(QString("✅ %1").arg(statusStr.toUpper()));
            m_profileStatus->setStyleSheet(QString(
                "QLabel#profileStatusBadge { font-size:12px; font-weight:800; background:%1; "
                "border:1.5px solid %2; border-radius:6px; padding:5px 8px; color:white; }")
                .arg(statusColor, cell.status == SeatStatus::Occupied ? "#15803d" : "#1d4ed8"));
            m_profileConflictDetail->setVisible(false);
        }

        // Disability Accessibility Tag
        if (st->isPhysicallyImpaired) {
            if (r == 0) {
                m_profileDisabilityTag->setText("♿ PHYSICALLY IMPAIRED (FRONT ROW ACCESSIBLE)");
                m_profileDisabilityTag->setStyleSheet(
                    "QLabel#profileDisabilityBadge { font-size:10px; font-weight:700; background:#eff6ff; "
                    "border:1.5px solid #93c5fd; color:#1e40af; border-radius:5px; padding:4px 6px; }");
            } else {
                m_profileDisabilityTag->setText("⚠️ DISABILITY CONFLICT (MUST BE IN FRONT ROW A)");
                m_profileDisabilityTag->setStyleSheet(
                    "QLabel#profileDisabilityBadge { font-size:10px; font-weight:800; background:#fff7ed; "
                    "border:1.5px solid #fdba74; color:#c2410c; border-radius:5px; padding:4px 6px; }");
            }
            m_profileDisabilityTag->setVisible(true);
        } else {
            m_profileDisabilityTag->setVisible(false);
        }

        m_profileRoll->setText(st->rollNumber.isEmpty() ? QString::number(st->id) : st->rollNumber);
        m_profileName->setText(st->name);
        m_profileDept->setText(st->department);
        m_profileSem->setText(QString("Semester %1").arg(st->semester));
        m_profileSec->setText(st->section.isEmpty() ? "—" : st->section);
        m_profileSub->setText(st->subject);
    } else {
        m_profileStatus->setText("AVAILABLE");
        m_profileStatus->setStyleSheet(
            "QLabel#profileStatusBadge { font-size:12px; font-weight:700; background:#f1f5f9; "
            "border:1.5px solid #cbd5e1; border-radius:6px; padding:5px 8px; color:#64748b; }");
        m_profileDisabilityTag->setVisible(false);
        m_profileConflictDetail->setVisible(false);
        m_profileRoll->setText("—");
        m_profileName->setText("—");
        m_profileDept->setText("—");
        m_profileSem->setText("—");
        m_profileSec->setText("—");
        m_profileSub->setText("—");
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Student Combo
// ─────────────────────────────────────────────────────────────────────────────

void SubBlockDialog::populateStudentCombo() {
    m_studentCombo->clear();
    m_studentCombo->addItem("— Select unassigned student —", -1);
    for (UIStudent *s : m_model->unassignedStudents()) {
        m_studentCombo->addItem(
            QString("[%1]  %2  ·  %3").arg(s->rollNumber, s->name, s->department),
            s->id
        );
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Slots
// ─────────────────────────────────────────────────────────────────────────────

void SubBlockDialog::onSeatCardClicked(int bench, int seat) {
    m_selectedBench = bench;
    m_selectedSeat  = seat;

    const SeatCell &cell = m_model->seatAt(m_block, m_sub, bench, seat);
    bool hasStudent = (cell.studentId != -1);

    m_assignBtn->setEnabled(!hasStudent && !cell.locked);
    m_unassignBtn->setEnabled(hasStudent && !cell.locked);
    m_lockBtn->setEnabled(true);
    m_lockBtn->setText(cell.locked ? "Unlock Seat" : "Lock Seat");

    // Sync table selection
    m_table->blockSignals(true);
    m_table->selectRow((bench - 1) * m_seatsPerBench + (seat - 1));
    m_table->blockSignals(false);

    refreshGrid();
    updateProfilePanel(bench, seat);
}

void SubBlockDialog::onCellClicked(int row, int) {
    int bench = (row / m_seatsPerBench) + 1;
    int seat  = (row % m_seatsPerBench) + 1;
    onSeatCardClicked(bench, seat);
}

void SubBlockDialog::onAssignClicked() {
    if (m_selectedBench < 1) return;
    int sid = m_studentCombo->currentData().toInt();
    if (sid < 0) { QMessageBox::warning(this, "No Student", "Please select a student first."); return; }
    if (!m_model->assignSeat(sid, m_block, m_sub, m_selectedBench, m_selectedSeat)) {
        QMessageBox::warning(this, "Failed", "Could not assign — seat may be locked or already occupied.");
        return;
    }
    refresh();
    populateStudentCombo();
    m_assignBtn->setEnabled(false);

    QStringList warnings = m_model->validateAndGetViolations();
    if (!warnings.isEmpty()) {
        QMessageBox::warning(
            this, "Rule Violation Warning",
            QString("WARNING: The manual assignment violates seating rule(s):\n\n• %1\n\nThe affected seat box(es) have been highlighted in warning color (red).")
                .arg(warnings.join("\n• ")));
    }
}

void SubBlockDialog::onUnassignClicked() {
    if (m_selectedBench < 1) return;
    m_model->unassignSeat(m_block, m_sub, m_selectedBench, m_selectedSeat);
    refresh();
    populateStudentCombo();
    m_assignBtn->setEnabled(false);
    m_unassignBtn->setEnabled(false);
}

void SubBlockDialog::onLockToggle() {
    if (m_selectedBench < 1) return;
    const SeatCell &cell = m_model->seatAt(m_block, m_sub, m_selectedBench, m_selectedSeat);
    m_model->lockSeat(m_block, m_sub, m_selectedBench, m_selectedSeat, !cell.locked);
    refresh();
    onSeatCardClicked(m_selectedBench, m_selectedSeat);
}

// ─────────────────────────────────────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────────────────────────────────────

QColor SubBlockDialog::statusBgColor(SeatStatus s) const {
    switch(s) {
        case SeatStatus::Occupied: return QColor("#dcfce7");
        case SeatStatus::Assigned: return QColor("#dbeafe");
        case SeatStatus::Reserved: return QColor("#fef9c3");
        case SeatStatus::Conflict: return QColor("#fee2e2");
        default:                   return QColor("#ffffff");
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
