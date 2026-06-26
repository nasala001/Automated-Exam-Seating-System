#pragma once
#include <QDialog>
#include <QTableWidget>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include "studentmodel.h"

class SubBlockDialog : public QDialog {
    Q_OBJECT
public:
    SubBlockDialog(const QString &block, int sub, HallModel *model, QWidget *parent=nullptr);

private slots:
    void onAssignClicked();
    void onUnassignClicked();
    void onLockToggle();
    void onCellClicked(int row, int col);
    void refreshTable();

private:
    QString    m_block;
    int        m_sub;
    HallModel *m_model;

    QLabel      *m_titleLbl;
    QLabel      *m_statsLbl;
    QTableWidget *m_table;
    QPushButton *m_assignBtn;
    QPushButton *m_unassignBtn;
    QPushButton *m_lockBtn;
    QComboBox   *m_StudentCombo;

    int m_selectedBench = -1;
    int m_selectedSeat  = -1;

    void buildUi();
    void populateStudentCombo();
    QColor statusColor(SeatStatus s) const;
    QString statusText(SeatStatus s) const;
};
