#include "../include/hallmap.h"
#include <QPainter>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QLinearGradient>
#include <QFont>
#include <QDialog>
#include <QComboBox>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QMessageBox>

// ── Color helpers ────────────────────────────────────────────────────────────
static QColor utilizationColor(double pct) {
    if (pct == 0)   return QColor("#f5f7fa");
    if (pct < 40)   return QColor("#d4edda");
    if (pct < 70)   return QColor("#fff3cd");
    if (pct < 90)   return QColor("#ffd0a0");
    return              QColor("#f8d7da");
}

// ═══════════════════════════════════════════════════════════════════════════
// SubBlockTile
// ═══════════════════════════════════════════════════════════════════════════
SubBlockTile::SubBlockTile(const QString &block, int sub, HallModel *model, QWidget *parent)
    : QWidget(parent), m_block(block), m_sub(sub), m_model(model)
{
    setFixedSize(60, 52);
    setCursor(Qt::PointingHandCursor);
    setToolTip(QString("%1%2 — click to view seats").arg(block).arg(sub));
}

void SubBlockTile::refresh() { update(); }

void SubBlockTile::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    int occ   = m_model->occupiedInSubBlock(m_block, m_sub);
    double pct= (double)occ / HallConst::PER_SUB * 100.0;

    QColor bg   = utilizationColor(pct);
    QColor border= m_hovered ? QColor("#003366") : QColor("#b0c4de");
    int bw = m_hovered ? 2 : 1;

    // Background
    p.setPen(QPen(border, bw));
    p.setBrush(bg);
    p.drawRoundedRect(rect().adjusted(1,1,-1,-1), 5, 5);

    // Sub-block label
    p.setPen(QColor("#1a2332"));
    QFont f("Segoe UI", 8, QFont::Bold);
    p.setFont(f);
    p.drawText(QRect(0, 4, width(), 18), Qt::AlignHCenter, QString("%1%2").arg(m_block).arg(m_sub));

    // Count
    p.setPen(QColor("#003366"));
    QFont f2("Segoe UI", 7);
    p.setFont(f2);
    p.drawText(QRect(0, 20, width(), 14), Qt::AlignHCenter,
               QString("%1/%2").arg(occ).arg(HallConst::PER_SUB));

    // Mini seat dots: 2 rows of 6
    int dotW = 6, dotH = 4, gap = 2;
    int totalW = 6*dotW + 5*gap;
    int startX = (width() - totalW) / 2;
    int startY = 36;
    for (int n=0; n<HallConst::BENCHES; n++) {
        for (int t=0; t<HallConst::SEATS_PER_BENCH; t++) {
            const SeatCell &cell = m_model->seatAt(m_block, m_sub, n+1, t+1);
            QColor dc;
            if (cell.locked) dc = QColor("#9ab0c8");
            else if (cell.studentId!=-1) {
                switch(cell.status) {
                    case SeatStatus::Occupied: dc=QColor("#28a745"); break;
                    case SeatStatus::Conflict: dc=QColor("#dc3545"); break;
                    default: dc=QColor("#0066cc"); break;
                }
            } else dc = QColor("#dee2e6");
            p.setPen(Qt::NoPen);
            p.setBrush(dc);
            int x = startX + n*(dotW+gap);
            int y = startY + t*(dotH+1);
            p.drawRoundedRect(x, y, dotW, dotH, 1, 1);
        }
    }
}

void SubBlockTile::mousePressEvent(QMouseEvent *) {
    emit clicked(m_block, m_sub);
}
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void SubBlockTile::enterEvent(QEnterEvent *) { m_hovered = true; update(); }
#else
void SubBlockTile::enterEvent(QEvent *) { m_hovered = true; update(); }
#endif
void SubBlockTile::leaveEvent(QEvent *)      { m_hovered=false; update(); }

// ═══════════════════════════════════════════════════════════════════════════
// MajorBlockWidget
// ═══════════════════════════════════════════════════════════════════════════
MajorBlockWidget::MajorBlockWidget(const QString &block, HallModel *model, QWidget *parent)
    : QWidget(parent), m_block(block), m_model(model)
{
    setMinimumSize(220, 130);
    auto *vl = new QVBoxLayout(this);
    vl->setContentsMargins(8,8,8,8);
    vl->setSpacing(4);

    // Sub-block tiles row
    auto *tileRow = new QHBoxLayout;
    tileRow->setSpacing(4);
    for (int sub=1; sub<=3; sub++) {
        auto *tile = new SubBlockTile(block, sub, model, this);
        connect(tile, &SubBlockTile::clicked, this, &MajorBlockWidget::subBlockClicked);
        m_tiles.append(tile);
        tileRow->addWidget(tile);
    }
    vl->addLayout(tileRow);
    setLayout(vl);
}

void MajorBlockWidget::refresh() {
    for (auto *t : m_tiles) t->refresh();
    update();
}

void MajorBlockWidget::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    int occ = m_model->occupiedInBlock(m_block);
    double pct = (double)occ / HallConst::PER_MAJOR * 100.0;

    // Outer card border
    QColor borderCol = QColor("#003366");
    p.setPen(QPen(borderCol, 1.5));
    p.setBrush(QColor("#ffffff"));
    p.drawRoundedRect(rect().adjusted(1,1,-1,-1), 8, 8);

    // Block header bar
    QRectF header(1.5, 1.5, width()-3, 28);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor("#003366"));
    p.drawRoundedRect(header, 7, 7);
    // square off bottom of header
    p.drawRect(QRectF(1.5, 16, width()-3, 13));

    // Block label
    p.setPen(Qt::white);
    QFont f("Segoe UI", 11, QFont::Bold);
    p.setFont(f);
    p.drawText(QRect(0, 2, width()/2, 26), Qt::AlignHCenter|Qt::AlignVCenter,
               QString("Block %1").arg(m_block));

    // Occupancy label
    p.setPen(QColor("#c9a84c"));
    QFont f2("Segoe UI", 8);
    p.setFont(f2);
    p.drawText(QRect(width()/2, 2, width()/2-4, 26), Qt::AlignRight|Qt::AlignVCenter,
               QString("%1/%2  %3%").arg(occ).arg(HallConst::PER_MAJOR).arg((int)pct));
}

// ── SeatCellWidget Implementation ───────────────────────────────────────────
SeatCellWidget::SeatCellWidget(int row, int col, HallModel *model, QWidget *parent)
    : QWidget(parent), m_row(row), m_col(col), m_model(model)
{
    setFixedSize(52, 38);
    setCursor(Qt::PointingHandCursor);
    refresh();
}

void SeatCellWidget::refresh() {
    const SeatCell &cell = m_model->seatAt(m_row, m_col);
    QString seatName = m_model->getSeatName(m_row, m_col);
    if (cell.studentId != -1) {
        UIStudent *st = m_model->findById(cell.studentId);
        if (st) {
            setToolTip(QString("Seat %1: %2 (%3)\n%4")
                       .arg(seatName).arg(st->name)
                       .arg(st->rollNumber).arg(st->subject));
        }
    } else {
        setToolTip(QString("Seat %1 (Empty)").arg(seatName));
    }
    update();
}

void SeatCellWidget::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const SeatCell &cell = m_model->seatAt(m_row, m_col);
    
    QColor bg;
    if (cell.locked) bg = QColor("#9ab0c8");
    else if (cell.studentId != -1) {
        switch(cell.status) {
            case SeatStatus::Occupied: bg = QColor("#28a745"); break;
            case SeatStatus::Conflict: bg = QColor("#dc3545"); break;
            default: bg = QColor("#0066cc"); break;
        }
    } else bg = QColor("#dee2e6");

    int highlightedId = -1;
    auto *mapWidget = qobject_cast<HallMapWidget*>(parentWidget() ? parentWidget()->parentWidget() : nullptr);
    if (mapWidget) {
        highlightedId = mapWidget->highlightedStudentId();
    }
    bool isHighlighted = (cell.studentId != -1 && cell.studentId == highlightedId);

    QColor border = m_hovered ? QColor("#003366") : QColor("#b0c4de");
    int bw = m_hovered ? 2 : 1;
    if (isHighlighted) {
        border = QColor("#c9a84c");
        bw = 3;
    }

    p.setPen(QPen(border, bw));
    p.setBrush(bg);
    p.drawRoundedRect(rect().adjusted(1,1,-1,-1), 4, 4);

    // Label
    p.setPen(cell.studentId != -1 || cell.locked ? Qt::white : QColor("#1a2332"));
    QFont f("Segoe UI", 7, QFont::Bold);
    p.setFont(f);
    QString label = m_model->getSeatName(m_row, m_col);
    // Break the label into two lines if it contains a hyphen to fit better
    if (label.contains("-")) {
        int lastHyphen = label.lastIndexOf("-");
        label.replace(lastHyphen, 1, "\n");
    }
    p.drawText(rect(), Qt::AlignCenter, label);
}

void SeatCellWidget::mousePressEvent(QMouseEvent *) {
    emit clicked(m_row, m_col);
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void SeatCellWidget::enterEvent(QEnterEvent *) { m_hovered = true; update(); }
#else
void SeatCellWidget::enterEvent(QEvent *) { m_hovered = true; update(); }
#endif
void SeatCellWidget::leaveEvent(QEvent *)      { m_hovered=false; update(); }


// ── HallMapWidget Implementation ────────────────────────────────────────────
HallMapWidget::HallMapWidget(HallModel *model, QWidget *parent)
    : QWidget(parent), m_model(model)
{
    m_mainVL = new QVBoxLayout(this);
    m_mainVL->setContentsMargins(0,0,0,0);
    m_mainVL->setSpacing(8);

    // FRONT label
    auto *frontLbl = new QLabel("▼  FRONT OF ROOM / BLACKBOARD  ▼");
    frontLbl->setAlignment(Qt::AlignCenter);
    frontLbl->setStyleSheet("font-size:11px; font-weight:bold; color:#003366; "
                            "background:#e8f0fb; border:1px solid #b0c4de; "
                            "border-radius:4px; padding:4px 12px;");
    m_mainVL->addWidget(frontLbl);

    m_gridContainer = new QWidget(this);
    m_mainVL->addWidget(m_gridContainer);

    // BACK label
    auto *backLbl = new QLabel("▲  BACK OF ROOM / EXIT  ▲");
    backLbl->setAlignment(Qt::AlignCenter);
    backLbl->setStyleSheet("font-size:11px; font-weight:bold; color:#6b7a8d; "
                           "background:#f5f7fa; border:1px solid #d0d9e8; "
                           "border-radius:4px; padding:4px 12px;");
    m_mainVL->addWidget(backLbl);

    // Legend
    auto *legendRow = new QHBoxLayout;
    legendRow->setSpacing(12);
    legendRow->addStretch();

    auto addLegend = [&](const QString &color, const QString &label) {
        auto *dot = new QLabel;
        dot->setFixedSize(12,12);
        dot->setStyleSheet(QString("background:%1; border-radius:6px;").arg(color));
        auto *lbl = new QLabel(label);
        lbl->setStyleSheet("font-size:11px; color:#6b7a8d;");
        legendRow->addWidget(dot);
        legendRow->addWidget(lbl);
    };
    addLegend("#28a745","Occupied");
    addLegend("#0066cc","Assigned");
    addLegend("#ffc107","Reserved");
    addLegend("#dc3545","Conflict");
    addLegend("#dee2e6","Empty");
    legendRow->addStretch();
    m_mainVL->addLayout(legendRow);
    setLayout(m_mainVL);
}

void HallMapWidget::refresh() {
    // Clear old container contents
    if (m_gridContainer->layout()) {
        QLayoutItem *item;
        while ((item = m_gridContainer->layout()->takeAt(0)) != nullptr) {
            delete item->widget();
            delete item;
        }
        delete m_gridContainer->layout();
    }

    m_blocks.clear();
    m_seats.clear();

    if (m_model->activeRoomID() == "MAIN_ROOM") {
        auto *grid = new QGridLayout(m_gridContainer);
        grid->setSpacing(10);
        const QStringList &bnames = HallConst::BLOCK_NAMES;
        for (int row=0; row<3; row++) {
            for (int col=0; col<3; col++) {
                int idx = row*3 + col;
                auto *mb = new MajorBlockWidget(bnames[idx], m_model, m_gridContainer);
                connect(mb, &MajorBlockWidget::subBlockClicked,
                        this, &HallMapWidget::subBlockClicked);
                m_blocks.append(mb);
                grid->addWidget(mb, row, col);
            }
        }
        m_gridContainer->setLayout(grid);
    } else {
        auto *grid = new QGridLayout(m_gridContainer);
        grid->setSpacing(6);
        int rows = m_model->rows();
        int cols = m_model->cols();
        for (int r = 0; r < rows; ++r) {
            for (int c = 0; c < cols; ++c) {
                auto *sc = new SeatCellWidget(r, c, m_model, m_gridContainer);
                connect(sc, &SeatCellWidget::clicked, this, &HallMapWidget::onSeatClicked);
                m_seats.append(sc);
                grid->addWidget(sc, r, c);
            }
        }
        m_gridContainer->setLayout(grid);
    }
}

void HallMapWidget::onSeatClicked(int row, int col) {
    const SeatCell &cell = m_model->seatAt(row, col);
    
    QDialog dialog(this);
    dialog.setWindowTitle(QString("Seat R%1-C%2 Management").arg(row+1).arg(col+1));
    auto *form = new QVBoxLayout(&dialog);

    QLabel *infoLbl = new QLabel(&dialog);
    infoLbl->setStyleSheet("font-size:13px; font-weight:bold; color:#003366;");
    form->addWidget(infoLbl);

    QComboBox *studentCombo = nullptr;
    QPushButton *unassignBtn = nullptr;
    QPushButton *lockBtn = nullptr;

    if (cell.studentId != -1) {
        UIStudent *st = m_model->findById(cell.studentId);
        if (st) {
            infoLbl->setText(QString("Assigned: %1 (%2)\nProgram: %3 | Subject: %4")
                             .arg(st->name).arg(st->rollNumber)
                             .arg(st->program).arg(st->subject));
            if (cell.status == SeatStatus::Conflict || st->status == SeatStatus::Conflict) {
                QString violation = m_model->getViolationDetailForSeat(row, col);
                auto *conflictBanner = new QLabel(QString("⚠️ CONFLICT: %1").arg(violation), &dialog);
                conflictBanner->setWordWrap(true);
                conflictBanner->setStyleSheet(
                    "background-color: #fee2e2; color: #991b1b; border: 1.5px solid #fca5a5; "
                    "border-radius: 6px; padding: 10px; font-weight: bold; font-size: 12px; margin-bottom: 8px;");
                form->addWidget(conflictBanner);
            }
        }
        
        unassignBtn = new QPushButton("Unassign Student", &dialog);
        unassignBtn->setStyleSheet("background-color: #dc3545; color: white; padding: 8px; font-weight: bold;");
        form->addWidget(unassignBtn);
    } else {
        infoLbl->setText(cell.locked ? "Seat is currently Locked." : "Seat is Empty.");
        
        if (!cell.locked) {
            studentCombo = new QComboBox(&dialog);
            studentCombo->addItem("-- Select Student --", -1);
            for (auto *st : m_model->unassignedStudents()) {
                studentCombo->addItem(QString("%1 (%2)").arg(st->name).arg(st->rollNumber), st->id);
            }
            form->addWidget(new QLabel("Assign Student:", &dialog));
            form->addWidget(studentCombo);
        }
    }

    lockBtn = new QPushButton(cell.locked ? "Unlock Seat" : "Lock Seat", &dialog);
    lockBtn->setStyleSheet("background-color: #6b7a8d; color: white; padding: 8px; font-weight: bold;");
    form->addWidget(lockBtn);

    auto *bbox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    form->addWidget(bbox);
    connect(bbox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(bbox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (unassignBtn) {
        connect(unassignBtn, &QPushButton::clicked, this, [&]() {
            m_model->unassignSeat(row, col);
            dialog.accept();
        });
    }
    
    connect(lockBtn, &QPushButton::clicked, this, [&]() {
        m_model->lockSeat(row, col, !cell.locked);
        dialog.accept();
    });

    if (dialog.exec() == QDialog::Accepted) {
        if (studentCombo && studentCombo->currentIndex() > 0) {
            int studentId = studentCombo->currentData().toInt();
            m_model->assignSeat(studentId, row, col);
            QStringList warnings = m_model->validateAndGetViolations();
            if (!warnings.isEmpty()) {
                QMessageBox::warning(
                    this, "Rule Violation Warning",
                    QString("WARNING: The manual assignment violates seating rule(s):\n\n• %1\n\nThe affected seat box(es) have been highlighted in warning color (red).")
                        .arg(warnings.join("\n• ")));
            }
        }
        refresh();
    }
}

void HallMapWidget::highlightStudent(int studentId) {
    m_highlightstudentId = studentId;
    UIStudent *st = m_model->findById(studentId);
    if (st && st->isAssigned()) {
        if (!st->block.isEmpty()) {
            emit subBlockClicked(st->block, st->subBlock);
        } else {
            refresh();
        }
    }
}
