#include "mainwindow.h"
#include "pages/dashboardpage.h"
#include "pages/studentpage.h"
#include "pages/roompage.h"
#include "pages/resultpage.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QFrame>

// ── Shared data accessors ──────────────────────────────────────────────────

std::vector<Student>& MainWindow::getStudents() { return students; }

void MainWindow::setStudents(std::vector<Student> s) {
    students = std::move(s);
    refreshDashboard();
}

std::vector<std::unique_ptr<Venue>>& MainWindow::getVenues() { return venues; }

void MainWindow::addVenue(std::unique_ptr<Venue> v) {
    venues.push_back(std::move(v));
    refreshDashboard();
}

int MainWindow::totalSeatCount() const {
    int total = 0;
    for (auto& v : venues) total += v->totalSeats();
    return total;
}

void MainWindow::refreshDashboard() {
    dashboardPage->updateStats(
        static_cast<int>(students.size()),
        static_cast<int>(venues.size()),
        totalSeatCount(),
        0  // conflicts — Phase 2
    );
}

// ── Constructor ────────────────────────────────────────────────────────────

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Exam Seating System");
    setMinimumSize(1100, 700);

    // Central widget
    QWidget *central = new QWidget(this);
    QHBoxLayout *root = new QHBoxLayout(central);
    root->setSpacing(0);
    root->setContentsMargins(0, 0, 0, 0);

    // ── Sidebar ──────────────────────────────
    QWidget *sidebar = new QWidget(this);
    sidebar->setFixedWidth(220);
    sidebar->setStyleSheet("background-color: #1e1e2e;");

    QVBoxLayout *sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->setContentsMargins(0, 0, 0, 0);
    sidebarLayout->setSpacing(0);

    // App title
    QLabel *appTitle = new QLabel("  🎓 Exam Seating");
    appTitle->setFixedHeight(60);
    appTitle->setStyleSheet(
        "color: #89b4fa; font-size: 15px; font-weight: bold;"
        "background-color: #181825; padding-left: 10px;"
        );
    sidebarLayout->addWidget(appTitle);

    // Nav buttons
    setupSidebar(sidebarLayout);
    sidebarLayout->addStretch();

    // ── Pages ────────────────────────────────
    pages = new QStackedWidget(this);
    pages->setStyleSheet("background-color: #181825;");
    setupPages();

    root->addWidget(sidebar);
    root->addWidget(pages);
    setCentralWidget(central);

    // Show dashboard by default
    showDashboard();
}

MainWindow::~MainWindow() {}

void MainWindow::setupSidebar(QVBoxLayout *sidebarLayout)
{
    QString btnStyle =
        "QPushButton {"
        "  background: transparent;"
        "  color: #cdd6f4;"
        "  padding: 14px 20px;"
        "  text-align: left;"
        "  border: none;"
        "  font-size: 14px;"
        "}"
        "QPushButton:hover {"
        "  background: #313244;"
        "  border-left: 3px solid #89b4fa;"
        "}"
        "QPushButton:checked {"
        "  background: #313244;"
        "  border-left: 3px solid #89b4fa;"
        "  color: #89b4fa;"
        "  font-weight: bold;"
        "}";

    btnDashboard = new QPushButton("  🏠  Dashboard", this);
    btnStudents  = new QPushButton("  👤  Students",  this);
    btnRooms     = new QPushButton("  🏫  Rooms",     this);
    btnResults   = new QPushButton("  📋  Results",   this);

    for (auto btn : {btnDashboard, btnStudents, btnRooms, btnResults}) {
        btn->setStyleSheet(btnStyle);
        btn->setCheckable(true);
        btn->setFixedHeight(50);
        sidebarLayout->addWidget(btn);
    }

    connect(btnDashboard, &QPushButton::clicked, this, &MainWindow::showDashboard);
    connect(btnStudents,  &QPushButton::clicked, this, &MainWindow::showStudents);
    connect(btnRooms,     &QPushButton::clicked, this, &MainWindow::showRooms);
    connect(btnResults,   &QPushButton::clicked, this, &MainWindow::showResults);
}

void MainWindow::setupPages()
{
    dashboardPage = new DashboardPage(this);
    studentPage   = new StudentPage(this);
    roomPage      = new RoomPage(this);
    resultPage    = new ResultPage(this);

    pages->addWidget(dashboardPage);   // index 0
    pages->addWidget(studentPage);     // index 1
    pages->addWidget(roomPage);        // index 2
    pages->addWidget(resultPage);      // index 3
}

void MainWindow::setActiveButton(QPushButton *active)
{
    for (auto btn : {btnDashboard, btnStudents, btnRooms, btnResults})
        btn->setChecked(false);
    active->setChecked(true);
}

void MainWindow::showDashboard() { pages->setCurrentIndex(0); setActiveButton(btnDashboard); }
void MainWindow::showStudents()  { pages->setCurrentIndex(1); setActiveButton(btnStudents);  }
void MainWindow::showRooms()     { pages->setCurrentIndex(2); setActiveButton(btnRooms);     }
void MainWindow::showResults()   { pages->setCurrentIndex(3); setActiveButton(btnResults);   }