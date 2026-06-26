#include "../include/analyticspage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFrame>
#include <QScrollArea>
#include <QTableWidget>
#include <QHeaderView>
#include <QPainter>
#include <QFont>

AnalyticsPage::AnalyticsPage(HallModel *model, QWidget *parent)
    : QWidget(parent), m_model(model)
{
    buildUi();
}

void AnalyticsPage::buildUi() {
    auto *vl = new QVBoxLayout(this);
    vl->setContentsMargins(28,24,28,24); vl->setSpacing(16);

    auto *titleLbl = new QLabel("Capacity Analytics");
    titleLbl->setStyleSheet("font-size:22px; font-weight:bold; color:#003366;");
    auto *subLbl = new QLabel("Real-time hall utilization, block distribution, and departmental breakdown");
    subLbl->setStyleSheet("font-size:12px; color:#6b7a8d;");
    vl->addWidget(titleLbl);
    vl->addWidget(subLbl);

    auto *sep = new QFrame; sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("background:#d0d9e8;"); sep->setFixedHeight(1);
    vl->addWidget(sep);

    // Hall heatmap section
    auto *heatTitle = new QLabel("Hall Occupancy Heatmap");
    heatTitle->setStyleSheet("font-size:15px; font-weight:bold; color:#003366;");
    vl->addWidget(heatTitle);

    auto *heatFrame = new QFrame;
    heatFrame->setStyleSheet("QFrame { background:white; border:1px solid #e0e7ef; border-radius:8px; }");
    auto *heatGL = new QGridLayout(heatFrame);
    heatGL->setContentsMargins(16,16,16,16); heatGL->setSpacing(8);

    // Legend
    auto *legRow = new QHBoxLayout;
    legRow->setSpacing(16);
    legRow->addStretch();
    auto addLeg = [&](const QString &color, const QString &label) {
        auto *w = new QWidget; w->setFixedSize(16,12);
        w->setStyleSheet(QString("background:%1; border-radius:2px;").arg(color));
        auto *l = new QLabel(label); l->setStyleSheet("font-size:11px; color:#6b7a8d;");
        legRow->addWidget(w); legRow->addWidget(l);
    };
    addLeg("#d4edda","0-39%"); addLeg("#fff3cd","40-69%");
    addLeg("#ffd0a0","70-89%"); addLeg("#f8d7da","90-100%");
    legRow->addStretch();

    const QStringList &bnames = HallConst::BLOCK_NAMES;
    for (int i=0; i<9; i++) {
        auto *cell = new QFrame;
        cell->setMinimumSize(120, 70);
        cell->setObjectName(QString("heat_%1").arg(bnames[i]));
        cell->setStyleSheet("QFrame { background:#f5f7fa; border:1px solid #d0d9e8; border-radius:6px; }");
        auto *cl = new QVBoxLayout(cell);
        cl->setContentsMargins(8,6,8,6); cl->setSpacing(3);

        auto *blbl = new QLabel(QString("Block %1").arg(bnames[i]));
        blbl->setStyleSheet("font-size:11px; font-weight:bold; color:#003366; border:none; background:transparent;");
        blbl->setAlignment(Qt::AlignCenter);

        auto *bar = new QProgressBar;
        bar->setRange(0, HallConst::PER_MAJOR);
        bar->setValue(0);
        bar->setFixedHeight(8);
        bar->setTextVisible(false);
        bar->setObjectName(QString("hbar_%1").arg(bnames[i]));
        bar->setStyleSheet(
            "QProgressBar { background:#e0e7ef; border-radius:4px; border:none; }"
            "QProgressBar::chunk { background:qlineargradient(x1:0, y1:0, x2:1, y2:0, "
            "stop:0 #003366, stop:1 #1a4a7a); border-radius:4px; }");
        m_blockBars.append(bar);

        auto *infoLbl = new QLabel("0 / 36 (0%)");
        infoLbl->setStyleSheet("font-size:12px; color:#1a2332; border:none; background:transparent;");
        infoLbl->setAlignment(Qt::AlignCenter);
        m_blockLabels.append(infoLbl);

        cl->addWidget(blbl);
        cl->addWidget(bar);
        cl->addWidget(infoLbl);
        heatGL->addWidget(cell, i/3, i%3);
    }
    vl->addWidget(heatFrame);

    // Dept breakdown
    auto *deptTitle = new QLabel("Department Distribution");
    deptTitle->setStyleSheet("font-size:15px; font-weight:bold; color:#003366;");
    vl->addWidget(deptTitle);

    m_deptWidget = new QFrame;
    m_deptWidget->setStyleSheet("QFrame { background:white; border:1px solid #e0e7ef; border-radius:8px; }");
    auto *deptVL = new QVBoxLayout(m_deptWidget);
    deptVL->setContentsMargins(0,0,0,0);
    // table will be inside
    vl->addWidget(m_deptWidget, 1);

    setLayout(vl);
    refresh();
}

void AnalyticsPage::refresh() {
    bool isMainRoom = (m_model->activeRoomID() == "MAIN_ROOM");
    const QStringList &bnames = HallConst::BLOCK_NAMES;
    for (int i=0; i<9; i++) {
        int pct = 0;
        if (isMainRoom) {
            int occ = m_model->occupiedInBlock(bnames[i]);
            pct = (int)((double)occ/HallConst::PER_MAJOR*100);
            m_blockBars[i]->setValue(occ);
            m_blockLabels[i]->setText(QString("%1 / %2  (%3%)")
                                       .arg(occ).arg(HallConst::PER_MAJOR).arg(pct));
        } else {
            m_blockBars[i]->setValue(0);
            m_blockLabels[i]->setText("N/A");
        }

        // Update heatmap cell color
        QColor bg;
        if (pct==0)      bg=QColor("#f5f7fa");
        else if (pct<40) bg=QColor("#d4edda");
        else if (pct<70) bg=QColor("#fff3cd");
        else if (pct<90) bg=QColor("#ffd0a0");
        else             bg=QColor("#f8d7da");
        auto cells = findChildren<QFrame*>(QString("heat_%1").arg(bnames[i]));
        for (auto *c : cells)
            c->setStyleSheet(QString("QFrame { background:%1; border:1px solid #d0d9e8; border-radius:6px; }")
                             .arg(bg.name()));
    }
    refreshDeptTable();
}

void AnalyticsPage::refreshDeptTable() {
    // Clear existing table
    auto *old = m_deptWidget->findChild<QTableWidget*>();
    if (old) delete old;

    QStringList depts = m_model->departments();
    if (depts.isEmpty()) return;

    auto *tbl = new QTableWidget(depts.size(), 4, m_deptWidget);
    tbl->setHorizontalHeaderLabels({"Department","Total Students","Assigned","Unassigned"});
    tbl->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    tbl->setSelectionBehavior(QAbstractItemView::SelectRows);
    tbl->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tbl->setAlternatingRowColors(true);
    tbl->verticalHeader()->hide();
    tbl->setStyleSheet(
        "QTableWidget { border:none; font-size:12px; }"
        "QHeaderView::section { background:#003366; color:white; font-weight:bold; "
        "padding:8px 10px; border:none; border-right:1px solid #1a4a7a; }"
        "QTableWidget::item:selected { background:#dce8f7; color:#003366; }"
        "QTableWidget::item:alternate { background:#f8fafc; }");

    for (int i=0; i<depts.size(); i++) {
        QList<UIStudent*> ds = m_model->studentsByDept(depts[i]);
        int total    = ds.size();
        int assigned = 0;
        for (UIStudent *s : ds) if (s->isAssigned()) assigned++;
        int unassigned = total - assigned;

        auto setC = [&](int col, const QString &text, Qt::Alignment al=Qt::AlignCenter) {
            auto *item = new QTableWidgetItem(text);
            item->setTextAlignment(al);
            tbl->setItem(i, col, item);
        };
        setC(0, depts[i], Qt::AlignVCenter|Qt::AlignLeft);
        setC(1, QString::number(total));
        auto *ai = new QTableWidgetItem(QString::number(assigned));
        ai->setTextAlignment(Qt::AlignCenter);
        if (assigned > 0) ai->setForeground(QColor("#155724"));
        tbl->setItem(i, 2, ai);
        auto *ui = new QTableWidgetItem(QString::number(unassigned));
        ui->setTextAlignment(Qt::AlignCenter);
        if (unassigned > 0) ui->setForeground(QColor("#856404"));
        tbl->setItem(i, 3, ui);
    }
    auto *deptVL = qobject_cast<QVBoxLayout*>(m_deptWidget->layout());
    if (deptVL) deptVL->addWidget(tbl);
}
