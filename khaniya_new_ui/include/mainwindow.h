#pragma once
#include <QMainWindow>
#include <QStackedWidget>
#include <QLabel>
#include <QPushButton>
#include "studentmodel.h"
#include "dashboardpage.h"
#include "Studentspage.h"
#include "seatingpage.h"
#include "analyticspage.h"
#include "exportmanager.h"
#include "roommanagerpage.h"

struct sqlite3;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void navigateTo(int index);
    void onModelChanged();
    void onExportCSV();
    void onExportHTML();
    void onExportBlockReport();
    void onLoadSampleData();
    void onSaveData();
    void onLoadData();
    void promptRoomSelection();

private:
    sqlite3*      m_db = nullptr;
    HallModel    *m_model;
    ExportManager *m_exporter;

    QWidget       *m_sidebar;
    QStackedWidget *m_stack;
    QList<QPushButton*> m_navBtns;
    QLabel        *m_statusBarLbl;

    DashboardPage *m_dashboard;
    StudentsPage  *m_StudentsPage;
    SeatingPage   *m_seatingPage;
    AnalyticsPage *m_analyticsPage;
    RoomManagerPage *m_roomManagerPage;

    void buildSidebar();
    void buildStatusBar();
    QPushButton* makeNavButton(const QString &icon, const QString &label, int index);
    void refreshAll();
};
