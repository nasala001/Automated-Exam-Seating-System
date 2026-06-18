#pragma once
#include <QWidget>
#include <QLabel>
#include <QProgressBar>
#include "studentmodel.h"

class AnalyticsPage : public QWidget {
    Q_OBJECT
public:
    explicit AnalyticsPage(HallModel *model, QWidget *parent=nullptr);
    void refresh();

private:
    HallModel *m_model;
    QList<QProgressBar*> m_blockBars;
    QList<QLabel*>       m_blockLabels;
    QLabel *m_deptTable;
    QWidget *m_deptWidget;

    void buildUi();
    void refreshDeptTable();
};
