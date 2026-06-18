#pragma once
#include <QWidget>
#include <QLabel>
#include "studentmodel.h"

class DashboardPage : public QWidget {
    Q_OBJECT
public:
    explicit DashboardPage(HallModel *model, QWidget *parent=nullptr);
    void refresh();

private:
    HallModel *m_model;
    QLabel *m_totalStudents;
    QLabel *m_assigned;
    QLabel *m_unassigned;
    QLabel *m_utilization;
    QLabel *m_capacity;
    QLabel *m_available;

    QWidget *makeStatCard(const QString &title, QLabel *&valueLbl,
                          const QString &color="#003366");
    QWidget *makeBlockGrid();
    void refreshBlockGrid();
    QList<QLabel*> m_blockLabels; // 9 labels for A-I
};
