// mainwindow.h
#pragma once
#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>

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

private slots:
    void showDashboard();
    void showStudents();
    void showRooms();
    void showResults();

private:
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