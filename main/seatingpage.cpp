#include "../include/seatingpage.h"
#include "../include/subblockdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QFrame>
#include <QTableWidget>
#include <QHeaderView>
#include <QScrollArea>
#include <QMessageBox>
#include <QFont>

SeatingPage::SeatingPage(HallModel *model, QWidget *parent)
    : QWidget(parent), m_model(model)
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
    auto *subLbl = new QLabel("Interactive hall map — click any sub-block to manage seat assignments");
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

    auto *autoBtn = new QPushButton("⚡ Auto-Assign All");
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

    tbl->addWidget(autoBtn);
    tbl->addWidget(clearBtn);
    tbl->addStretch();
    tbl->addWidget(m_statusLbl);
    vl->addWidget(toolbar);

    // Splitter: hall map (left) | search panel (right)
    auto *splitter = new QSplitter(Qt::Horizontal);
    splitter->setHandleWidth(6);
    splitter->setStyleSheet("QSplitter::handle { background:#d0d9e8; }");

    // Hall map in scroll area
    auto *mapFrame = new QFrame;
    mapFrame->setStyleSheet("QFrame { background:white; border:1px solid #e0e7ef; border-radius:8px; }");
    auto *mapVL = new QVBoxLayout(mapFrame);
    mapVL->setContentsMargins(12,12,12,12);

    auto *mapTitle = new QLabel("Hall Map — KU Examination Hall");
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
    m_searchBox->setPlaceholderText("🔍  Name or roll number...");
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

void SeatingPage::refresh() {
    m_hallMap->refresh();
    int occ   = m_model->occupiedCount();
    int total = m_model->allStudents().size();
    m_statusLbl->setText(QString("%1 / %2 students assigned  (%3 / %4 seats)")
                         .arg(occ).arg(total)
                         .arg(occ).arg(HallConst::CAPACITY));
    onSearch();
}

void SeatingPage::onAutoAssign() {
    int before = m_model->occupiedCount();
    m_model->autoAssign();
    int after  = m_model->occupiedCount();
    refresh();
    m_statusLbl->setText(QString("Auto-assigned %1 students. Total: %2/%3")
                         .arg(after-before).arg(after).arg(HallConst::CAPACITY));
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
    QList<Student*> results = q.isEmpty() ? m_model->allStudents()
                                           : m_model->searchStudents(q);
    m_searchResults->setRowCount(0);
    for (Student *s : results) {
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
        setC(3, s->isAssigned() ? s->seatCode() : "—");
    }
}

void SeatingPage::onSearchResultClicked(int row, int) {
    auto *item = m_searchResults->item(row, 0);
    if (!item) return;
    int sid = item->data(Qt::UserRole).toInt();
    m_hallMap->highlightStudent(sid);
    Student *s = m_model->findById(sid);
    if (s && s->isAssigned()) {
        m_statusLbl->setText(QString("Showing: %1 → %2").arg(s->name, s->seatCode()));
    }
}

void SeatingPage::jumpToStudent(int studentId) {
    m_hallMap->highlightStudent(studentId);
}
