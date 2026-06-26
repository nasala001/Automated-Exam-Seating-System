#include "../include/dashboardpage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFrame>
#include <QProgressBar>
#include <QFont>
#include <QScrollArea>

DashboardPage::DashboardPage(HallModel *model, QWidget *parent)
    : QWidget(parent), m_model(model)
{
    auto *vl = new QVBoxLayout(this);
    vl->setContentsMargins(28, 24, 28, 24);
    vl->setSpacing(20);

    // Page header
    auto *titleLbl = new QLabel("Dashboard");
    titleLbl->setStyleSheet("font-size:22px; font-weight:bold; color:#003366;");
    auto *subLbl = new QLabel("Kathmandu University — Examination Hall Occupancy Overview");
    subLbl->setStyleSheet("font-size:12px; color:#6b7a8d;");
    vl->addWidget(titleLbl);
    vl->addWidget(subLbl);

    auto *sep = new QFrame; sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("background:#d0d9e8;"); sep->setFixedHeight(1);
    vl->addWidget(sep);

    // Stat cards row
    auto *cardsRow = new QHBoxLayout;
    cardsRow->setSpacing(16);
    cardsRow->addWidget(makeStatCard("Total Students",  m_totalStudents,  "#003366"));
    cardsRow->addWidget(makeStatCard("Assigned Seats",  m_assigned,       "#155724"));
    cardsRow->addWidget(makeStatCard("Unassigned",      m_unassigned,     "#856404"));
    cardsRow->addWidget(makeStatCard("Hall Capacity",   m_capacity,       "#004085"));
    cardsRow->addWidget(makeStatCard("Available Seats", m_available,      "#1a5c2e"));
    cardsRow->addWidget(makeStatCard("Utilization %",   m_utilization,    "#5a1a7a"));
    vl->addLayout(cardsRow);

    // Block overview grid
    auto *gridTitle = new QLabel("Block-wise Occupancy");
    gridTitle->setStyleSheet("font-size:15px; font-weight:bold; color:#003366; margin-top:8px;");
    vl->addWidget(gridTitle);
    vl->addWidget(makeBlockGrid());

    // ── Seating Map section ──
    auto *sep2 = new QFrame; sep2->setFrameShape(QFrame::HLine);
    sep2->setStyleSheet("background:#d0d9e8;"); sep2->setFixedHeight(1);
    vl->addWidget(sep2);

    auto *mapTitle = new QLabel("Seating Map");
    mapTitle->setStyleSheet("font-size:15px; font-weight:bold; color:#003366; margin-top:8px;");
    vl->addWidget(mapTitle);

    m_hallMap = new HallMapWidget(m_model, this);
    connect(m_hallMap, &HallMapWidget::subBlockClicked, this, &DashboardPage::subBlockClicked);

    auto *scrollArea = new QScrollArea(this);
    scrollArea->setWidget(m_hallMap);
    scrollArea->setWidgetResizable(true);
    scrollArea->setMinimumHeight(300);
    scrollArea->setStyleSheet("QScrollArea { border: 1px solid #d0d9e8; border-radius: 8px; background: white; }");
    vl->addWidget(scrollArea, 1);

    setLayout(vl);
    refresh();
}

QWidget* DashboardPage::makeStatCard(const QString &title, QLabel *&valueLbl,
                                     const QString &color) {
    auto *card = new QFrame;
    card->setStyleSheet(QString(
        "QFrame { background:white; border:1px solid #e0e7ef; border-radius:8px; "
        "border-left:4px solid %1; }").arg(color));
    card->setMinimumWidth(130);

    auto *cl = new QVBoxLayout(card);
    cl->setContentsMargins(16,14,16,14);
    cl->setSpacing(4);

    valueLbl = new QLabel("0");
    valueLbl->setStyleSheet(QString("font-size:26px; font-weight:bold; color:%1; "
                                    "border:none; background:transparent;").arg(color));

    auto *lbl = new QLabel(title);
    lbl->setStyleSheet("font-size:11px; color:#6b7a8d; border:none; background:transparent;");
    lbl->setWordWrap(true);

    cl->addWidget(valueLbl);
    cl->addWidget(lbl);
    return card;
}

QWidget* DashboardPage::makeBlockGrid() {
    auto *frame = new QFrame;
    frame->setStyleSheet("QFrame { background:white; border:1px solid #e0e7ef; border-radius:8px; }");
    auto *gl = new QGridLayout(frame);
    gl->setContentsMargins(16,16,16,16);
    gl->setSpacing(10);

    const QStringList &bnames = HallConst::BLOCK_NAMES;
    for (int i=0; i<9; i++) {
        auto *cell = new QFrame;
        cell->setStyleSheet("QFrame { background:#f5f7fa; border:1px solid #d0d9e8; border-radius:6px; }");
        auto *cl = new QVBoxLayout(cell);
        cl->setContentsMargins(10,8,10,8);
        cl->setSpacing(4);

        auto *blockLbl = new QLabel(QString("Block %1").arg(bnames[i]));
        blockLbl->setStyleSheet("font-size:11px; font-weight:bold; color:#003366; "
                                "border:none; background:transparent;");
        blockLbl->setAlignment(Qt::AlignCenter);

        auto *info = new QLabel("0 / 36");
        info->setStyleSheet("font-size:13px; color:#1a2332; border:none; background:transparent;");
        info->setAlignment(Qt::AlignCenter);
        m_blockLabels.append(info);

        auto *bar = new QProgressBar;
        bar->setRange(0, HallConst::PER_MAJOR);
        bar->setValue(0);
        bar->setFixedHeight(6);
        bar->setTextVisible(false);
        bar->setStyleSheet(
            "QProgressBar { background:#e0e7ef; border-radius:3px; border:none; }"
            "QProgressBar::chunk { background:#003366; border-radius:3px; }"
        );
        bar->setObjectName(QString("bar_%1").arg(bnames[i]));

        cl->addWidget(blockLbl);
        cl->addWidget(info);
        cl->addWidget(bar);

        gl->addWidget(cell, i/3, i%3);
    }
    return frame;
}

void DashboardPage::refresh() {
    int total    = m_model->allStudents().size();
    int occ      = m_model->occupiedCount();
    int unassign = total - occ;
    int capacity = m_model->rows() * m_model->cols();
    if (capacity == 0) capacity = HallConst::CAPACITY; // fallback

    m_totalStudents->setText(QString::number(total));
    m_assigned->setText(QString::number(occ));
    m_unassigned->setText(QString::number(unassign));
    m_capacity->setText(QString::number(capacity));
    m_available->setText(QString::number(capacity - occ));
    m_utilization->setText(QString("%1%").arg((int)m_model->utilizationPercent()));

    bool isMainRoom = (m_model->activeRoomID() == "MAIN_ROOM");
    
    // The grid might not be valid for non-MAIN_ROOM. We just hide/show the data or the whole widget.
    // Assuming m_blockLabels are children of the block grid, we can just update them if isMainRoom,
    // or set them to 0 if not.
    const QStringList &bnames = HallConst::BLOCK_NAMES;
    for (int i=0; i<9; i++) {
        if (isMainRoom) {
            int bocc = m_model->occupiedInBlock(bnames[i]);
            m_blockLabels[i]->setText(QString("%1 / %2").arg(bocc).arg(HallConst::PER_MAJOR));
            auto bars = findChildren<QProgressBar*>(QString("bar_%1").arg(bnames[i]));
            for (auto *b : bars) b->setValue(bocc);
        } else {
            m_blockLabels[i]->setText("N/A");
            auto bars = findChildren<QProgressBar*>(QString("bar_%1").arg(bnames[i]));
            for (auto *b : bars) b->setValue(0);
        }
    }

    // Refresh the seating map
    if (m_hallMap) m_hallMap->refresh();
}
