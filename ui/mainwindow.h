// mainwindow.h
#pragma once
#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "Student.h"
#include "Venue.h"

#include <vector>
#include <memory>

class DashboardPage;
class StudentPage;
class RoomPage;
class ResultPage;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // ── Shared data accessors ──────────────────
    std::vector<Student>&                       getStudents();
    void                                        setStudents(std::vector<Student> s);
    std::vector<std::unique_ptr<Venue>>&        getVenues();
    void                                        addVenue(std::unique_ptr<Venue> v);
    int                                         totalSeatCount() const;

public slots:
    void refreshDashboard();

private slots:
    void showDashboard();
    void showStudents();
    void showRooms();
    void showResults();

private:
    // ── Shared data ────────────────────────────
    std::vector<Student>                students;
    std::vector<std::unique_ptr<Venue>> venues;

    // ── UI ─────────────────────────────────────
    QStackedWidget *pages;

    QPushButton *btnDashboard;
    QPushButton *btnStudents;
    QPushButton *btnRooms;
    QPushButton *btnResults;

    DashboardPage *dashboardPage;
    StudentPage   *studentPage;
    RoomPage      *roomPage;
    ResultPage    *resultPage;

    void setupSidebar(QVBoxLayout *sidebarLayout);
    void setupPages();
    void setActiveButton(QPushButton *active);
};