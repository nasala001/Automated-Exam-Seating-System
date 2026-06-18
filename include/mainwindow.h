#pragma once
#include <QMainWindow>
#include <QStackedWidget>
#include <QLabel>
#include <QPushButton>
#include "studentmodel.h"
#include "dashboardpage.h"
#include "studentspage.h"
#include "seatingpage.h"
#include "analyticspage.h"
#include "exportmanager.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;

private slots:
    void navigateTo(int index);
    void onModelChanged();
    void onExportCSV();
    void onExportHTML();
    void onExportBlockReport();
    void onLoadSampleData();
    void onSaveData();
    void onLoadData();

private:
    HallModel    *m_model;
    ExportManager *m_exporter;

    QWidget       *m_sidebar;
    QStackedWidget *m_stack;
    QList<QPushButton*> m_navBtns;
    QLabel        *m_statusBarLbl;

    DashboardPage *m_dashboard;
    StudentsPage  *m_studentsPage;
    SeatingPage   *m_seatingPage;
    AnalyticsPage *m_analyticsPage;

    void buildSidebar();
    void buildStatusBar();
    QPushButton* makeNavButton(const QString &icon, const QString &label, int index);
    void refreshAll();
};
