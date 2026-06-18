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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("KU Examination Seating System");
    setMinimumSize(1200, 780);
    resize(1440, 900);

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
    m_studentsPage = new StudentsPage(m_model);
    m_seatingPage  = new SeatingPage(m_model);
    m_analyticsPage= new AnalyticsPage(m_model);

    m_stack->addWidget(m_dashboard);     // 0
    m_stack->addWidget(m_studentsPage);  // 1
    m_stack->addWidget(m_seatingPage);   // 2
    m_stack->addWidget(m_analyticsPage); // 3

    hl->addWidget(m_stack, 1);
    setCentralWidget(central);

    buildStatusBar();
    navigateTo(0);
    m_model->loadSampleData();
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

    auto *appTitle = new QLabel("ExamSeat Pro");
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
        {"▣", "Dashboard",   0},
        {"✎", "Students",    1},
        {"⊞", "Hall Seating",2},
        {"◑", "Analytics",   3},
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
    m_studentsPage->refresh();
    m_seatingPage->refresh();
    m_analyticsPage->refresh();
}

void MainWindow::onLoadSampleData() {
    if (QMessageBox::question(this,"Load Sample Data",
        "This will add sample students. Continue?",
        QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes) {
        m_model->loadSampleData();
    }
}

void MainWindow::onSaveData() {
    QString path = QFileDialog::getSaveFileName(this,"Save Data","kuseat_data.json",
                                                "JSON Files (*.json)");
    if (path.isEmpty()) return;
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly)) return;
    f.write(QJsonDocument(m_model->toJson()).toJson());
    QMessageBox::information(this,"Saved","Data saved successfully.");
}

void MainWindow::onLoadData() {
    QString path = QFileDialog::getOpenFileName(this,"Load Data","",
                                                "JSON Files (*.json)");
    if (path.isEmpty()) return;
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) return;
    QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    if (!doc.isObject()) { QMessageBox::warning(this,"Error","Invalid file."); return; }
    m_model->fromJson(doc.object());
    refreshAll();
    QMessageBox::information(this,"Loaded","Data loaded successfully.");
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
