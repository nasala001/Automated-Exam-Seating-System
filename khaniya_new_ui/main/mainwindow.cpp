#include "../include/mainwindow.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QFileDialog>
#include <QJsonDocument>
#include <QMessageBox>
#include <QApplication>
#include <QStatusBar>
#include <QFont>
#include <QLabel>
#include <QStyle>
#include <QTimer>
#include <QInputDialog>
#include <sqlite3.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("KU Seating System");
    setMinimumSize(1200, 780);
    resize(1440, 900);

    // Try multiple paths for DB
    QStringList dbPaths = {
        "data/exam_seating.db",
        "../data/exam_seating.db",
        "../../data/exam_seating.db"
    };
    
    for (const QString& p : dbPaths) {
        m_db = nasala::HallRepository::openDatabase(p.toStdString());
        if (m_db) {
            // Check if it's a valid DB or just created empty file
            // Let's just break on first successful open (SQLite will create it if not found, but we want it in the right directory)
            // It's better to check if directory exists
            QFileInfo fi(p);
            if (fi.dir().exists()) {
                break;
            } else {
                sqlite3_close(m_db);
                m_db = nullptr;
            }
        }
    }
    
    if (!m_db) {
        // Fallback to local execution dir
        m_db = nasala::HallRepository::openDatabase("exam_seating.db");
    }

    m_model    = new HallModel(this);
    m_exporter = new ExportManager(m_model, this);

    connect(m_model, &HallModel::dataChanged, this, &MainWindow::onModelChanged);

    // Central widget: sidebar + stack
    auto *central = new QWidget;
    auto *hl = new QHBoxLayout(central);
    hl->setContentsMargins(0,0,0,0);
    hl->setSpacing(0);

    buildSidebar();
    hl->addWidget(m_sidebar);

    // Pages
    m_stack = new QStackedWidget;
    m_dashboard    = new DashboardPage(m_model);
    m_StudentsPage = new StudentsPage(m_model);
    m_seatingPage  = new SeatingPage(m_model, m_db);
    m_analyticsPage= new AnalyticsPage(m_model);
    m_roomManagerPage = new RoomManagerPage(m_db);

    m_stack->addWidget(m_dashboard);     // 0
    m_stack->addWidget(m_StudentsPage);  // 1
    m_stack->addWidget(m_seatingPage);   // 2
    m_stack->addWidget(m_analyticsPage); // 3
    m_stack->addWidget(m_roomManagerPage); // 4

    connect(m_roomManagerPage, &RoomManagerPage::roomChanged, this, &MainWindow::refreshAll);

    hl->addWidget(m_stack, 1);
    setCentralWidget(central);

    buildStatusBar();
    navigateTo(0);
    m_model->loadSampleData();
    QTimer::singleShot(0, this, &MainWindow::promptRoomSelection);
}

MainWindow::~MainWindow() {
    if (m_db) {
        sqlite3_close(m_db);
    }
}

void MainWindow::buildSidebar() {
    m_sidebar = new QWidget;
    m_sidebar->setObjectName("sidebarWidget");
    m_sidebar->setFixedWidth(220);
    m_sidebar->setStyleSheet(
        "#sidebarWidget { background-color: #003366; border-right: 1px solid #00224d; }"
    );

    auto *vl = new QVBoxLayout(m_sidebar);
    vl->setContentsMargins(0,0,0,0);
    vl->setSpacing(0);

    // App Title
    auto *titleBar = new QWidget;
    titleBar->setObjectName("appTitleBar");
    titleBar->setStyleSheet("QWidget { background-color: #00224d; border-bottom: 2px solid #c9a84c; }");
    auto *tvl = new QVBoxLayout(titleBar);
    tvl->setContentsMargins(16, 16, 16, 14);
    tvl->setSpacing(3);

    auto *appTitle = new QLabel("Exam Seating");
    appTitle->setStyleSheet("font-size:16px; font-weight:bold; color:#c9a84c; "
                            "font-family:'Segoe UI',Arial,sans-serif;");
    auto *appSub = new QLabel("Kathmandu University");
    appSub->setStyleSheet("font-size:10px; color:#a0b4cc; "
                          "font-family:'Segoe UI',Arial,sans-serif;");
    tvl->addWidget(appTitle);
    tvl->addWidget(appSub);
    vl->addWidget(titleBar);

    // Nav section label
    auto addSectionLbl = [&](const QString &text) {
        auto *lbl = new QLabel(text);
        lbl->setStyleSheet("font-size:10px; font-weight:bold; color:#4a6a8a; "
                           "padding:16px 20px 4px 20px; letter-spacing:1px; "
                           "font-family:'Segoe UI',Arial,sans-serif; background:transparent;");
        vl->addWidget(lbl);
    };

    addSectionLbl("NAVIGATION");

    struct NavItem { QString icon; QString label; int page; };
    QList<NavItem> items = {
        {"-", "Dashboard",   0},
        {"oZ", "Students",    1},
        {"Sz", "Seat Assignment",2},
        {"-'", "Analytics",   3},
        {"S", "Room Manager",4},
    };
    for (auto &ni : items) {
        auto *btn = makeNavButton(ni.icon, ni.label, ni.page);
        m_navBtns.append(btn);
        vl->addWidget(btn);
    }

    addSectionLbl("DATA");
    auto addToolBtn = [&](const QString &label, auto slot) {
        auto *btn = new QPushButton(label);
        btn->setStyleSheet(
            "QPushButton { background:transparent; color:#b8cce0; text-align:left; border:none; "
            "border-left:3px solid transparent; padding:10px 20px; font-size:12px; "
            "font-family:'Segoe UI',Arial,sans-serif; }"
            "QPushButton:hover { background:#1a4a7a; color:white; border-left:3px solid #c9a84c; }");
        connect(btn, &QPushButton::clicked, this, slot);
        vl->addWidget(btn);
    };
    addToolBtn("📋  Load Sample Data", &MainWindow::onLoadSampleData);
    addToolBtn("💾  Save Data",         &MainWindow::onSaveData);
    addToolBtn("📂  Load Data",         &MainWindow::onLoadData);

    addSectionLbl("EXPORT");
    addToolBtn("📊  Export CSV",          &MainWindow::onExportCSV);
    addToolBtn("🖨   Seating Chart HTML", &MainWindow::onExportHTML);
    addToolBtn("📋  Block Report HTML",  &MainWindow::onExportBlockReport);

    vl->addStretch();

    // Footer
    auto *footer = new QWidget;
    footer->setStyleSheet("QWidget { background:#00224d; border-top:1px solid #1a4a7a; }");
    auto *fl = new QVBoxLayout(footer);
    fl->setContentsMargins(16,10,16,12); fl->setSpacing(2);

    auto *ftitle = new QLabel("KU ExamSeat v1.0");
    ftitle->setStyleSheet("font-size:11px; font-weight:bold; color:#c9a84c; background:transparent;");
    auto *fsub   = new QLabel("Office of the Controller\nof Examinations");
    fsub->setStyleSheet("font-size:9px; color:#4a6a8a; background:transparent;");
    fl->addWidget(ftitle);
    fl->addWidget(fsub);
    vl->addWidget(footer);
    m_sidebar->setLayout(vl);
}

QPushButton* MainWindow::makeNavButton(const QString &icon, const QString &label, int index) {
    auto *btn = new QPushButton(QString("  %1   %2").arg(icon, label));
    btn->setCheckable(true);
    btn->setStyleSheet(
        "QPushButton { background:transparent; color:#b8cce0; text-align:left; "
        "border:none; border-left:3px solid transparent; padding:12px 20px; "
        "font-size:13px; font-family:'Segoe UI',Arial,sans-serif; }"
        "QPushButton:hover { background:#1a4a7a; color:white; border-left:3px solid #c9a84c; }"
        "QPushButton:checked { background:#1a4a7a; color:white; font-weight:bold; "
        "border-left:3px solid #c9a84c; }");
    connect(btn, &QPushButton::clicked, this, [this, index]{ navigateTo(index); });
    return btn;
}

void MainWindow::buildStatusBar() {
    m_statusBarLbl = new QLabel("Ready — KU Examination Seating Management System");
    m_statusBarLbl->setStyleSheet("color:#b8cce0; font-size:11px; padding:0 8px;");
    statusBar()->addWidget(m_statusBarLbl);
    statusBar()->setStyleSheet(
        "QStatusBar { background:#003366; color:#b8cce0; font-size:11px; "
        "border-top:1px solid #00224d; }"
        "QStatusBar::item { border:none; }");
}

void MainWindow::navigateTo(int index) {
    m_stack->setCurrentIndex(index);
    for (int i=0; i<m_navBtns.size(); i++)
        m_navBtns[i]->setChecked(i == index);
    refreshAll();
}

void MainWindow::onModelChanged() { refreshAll(); }

void MainWindow::refreshAll() {
    int total = m_model->allStudents().size();
    int occ   = m_model->occupiedCount();
    m_statusBarLbl->setText(
        QString("Students: %1 | Assigned: %2 | Unassigned: %3 | Hall: %4/%5 (%6%)")
        .arg(total).arg(occ).arg(total-occ)
        .arg(occ).arg(HallConst::CAPACITY)
        .arg((int)m_model->utilizationPercent()));
    m_dashboard->refresh();
    m_StudentsPage->refresh();
    m_seatingPage->refresh();
    m_roomManagerPage->refresh();
    m_analyticsPage->refresh();
}

void MainWindow::onLoadSampleData() {
    if (QMessageBox::question(this,"Load Sample Data",
        "This will add sample Students. Continue?",
        QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes) {
        m_model->loadSampleData();
    }
}

void MainWindow::onSaveData() {
    QString path = QFileDialog::getSaveFileName(this,"Save Data","kuseat_data.csv",
                                                "CSV Files (*.csv)");
    if (path.isEmpty()) return;
    m_model->saveToCSV(path);
    QMessageBox::information(this,"Saved","Data saved as CSV successfully.");
}

void MainWindow::onLoadData() {
    QString path = QFileDialog::getOpenFileName(this,"Load Data","","CSV Files (*.csv)");
    if (path.isEmpty()) return;
    m_model->loadFromCSV(path);
    refreshAll();
    QMessageBox::information(this,"Loaded","Data loaded from CSV successfully.");
}

struct RoomInfo { QString hallId; QString roomId; QString hallName; int rows; int cols; };

void MainWindow::promptRoomSelection() {
    if (!m_db) return;

    QStringList roomNames;
    QList<RoomInfo> rooms;

    nasala::HallRepository hallRepo(m_db);
    auto halls = hallRepo.loadAll();
    for (const auto& hall : halls) {
        for (const auto& room : hall.getRooms()) {
            roomNames << QString("%1 - %2 (%3x%4)")
                             .arg(QString::fromStdString(hall.getHallName()))
                             .arg(QString::fromStdString(room.getRoomName()))
                             .arg(room.getRows())
                             .arg(room.getColumns());
            rooms.append({QString::fromStdString(hall.getHallID()), QString::fromStdString(room.getRoomID()), QString::fromStdString(hall.getHallName()), room.getRows(), room.getColumns()});
        }
    }

    if (roomNames.isEmpty()) {
        QMessageBox::information(this, "No Rooms", "Please add a room in the Room Manager first.");
        navigateTo(4); // Room Manager
        return;
    }

    bool ok;
    QString item = QInputDialog::getItem(this, "Select Room", "Choose a room to begin:", roomNames, 0, false, &ok);
    if (ok && !item.isEmpty()) {
        int idx = roomNames.indexOf(item);
        if (idx >= 0) {
            m_model->setActiveRoom(rooms[idx].hallId, rooms[idx].roomId, rooms[idx].rows, rooms[idx].cols, rooms[idx].hallName);
            refreshAll();
        }
    }
}

void MainWindow::onExportCSV() {
    QString path = QFileDialog::getSaveFileName(this,"Export CSV","kuseat_export.csv",
                                                "CSV Files (*.csv)");
    if (!path.isEmpty() && m_exporter->exportCSV(path))
        QMessageBox::information(this,"Exported","CSV exported successfully.");
}

void MainWindow::onExportHTML() {
    QString path = QFileDialog::getSaveFileName(this,"Export Seating Chart",
                                                "kuseat_chart.html","HTML Files (*.html)");
    if (!path.isEmpty() && m_exporter->exportHTML(path)) {
        QMessageBox::information(this,"Exported",
                                 "HTML seating chart exported.\nOpen in a browser to print.");
    }
}

void MainWindow::onExportBlockReport() {
    QString path = QFileDialog::getSaveFileName(this,"Export Block Report",
                                                "kuseat_block_report.html","HTML Files (*.html)");
    if (!path.isEmpty() && m_exporter->exportBlockReport(path))
        QMessageBox::information(this,"Exported","Block report exported successfully.");
}
