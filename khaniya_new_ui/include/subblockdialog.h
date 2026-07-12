#pragma once
#include <QDialog>
#include <QTableWidget>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QTabWidget>
#include <QScrollArea>
#include <QFrame>
#include "studentmodel.h"

// ── Visual Seat Card Widget ───────────────────────────────────────────────────
// A rich interactive card for a single seat in the visual grid view
class SeatCard : public QFrame {
    Q_OBJECT
public:
    SeatCard(const QString &seatCode, int bench, int seat, QWidget *parent = nullptr);
    void update(const SeatCell &cell, UIStudent *st, bool selected);
    QString statusText(SeatStatus s) const;

signals:
    void clicked(int bench, int seat);

protected:
    void mousePressEvent(QMouseEvent *) override;
    void enterEvent(QEnterEvent *) override;
    void leaveEvent(QEvent *) override;
    void paintEvent(QPaintEvent *) override;

private:
    QString m_seatCode;
    int m_bench;
    int m_seat;
    bool m_hovered = false;
    bool m_selected = false;

    QLabel *m_codeLabel;     // seat code top-left  e.g. A1-B2-S1
    QLabel *m_rollLabel;     // large roll number center
    QLabel *m_nameLabel;     // student name (truncated)
    QLabel *m_statusLabel;   // status badge bottom

    void applyStyle(const SeatCell &cell);
};

// ── Sub-Block Dialog (Redesigned) ─────────────────────────────────────────────
class SubBlockDialog : public QDialog {
    Q_OBJECT
public:
    // Generic constructor – works for MAIN_ROOM block layout
    SubBlockDialog(const QString &block, int sub, HallModel *model, QWidget *parent = nullptr);

private slots:
    void onAssignClicked();
    void onUnassignClicked();
    void onLockToggle();
    void onCellClicked(int row, int col);        // from table tab
    void onSeatCardClicked(int bench, int seat); // from visual tab
    void refresh();

private:
    QString    m_block;
    int        m_sub;
    HallModel *m_model;
    int        m_numBenches;
    int        m_seatsPerBench;

    // ── UI Elements ──
    QLabel    *m_titleLbl;
    QLabel    *m_statsLbl;
    QTabWidget *m_tabs;

    // Tab 1: Visual Grid
    QWidget    *m_visualTab;
    QList<QList<SeatCard*>> m_seatCards;   // [bench-1][seat-1]
    QScrollArea *m_gridScroll;

    // Side Panel – Student Profile Card
    QFrame *m_profileCard;
    QLabel *m_profileSeatCode;
    QLabel *m_profileStatus;
    QLabel *m_profileRoll;
    QLabel *m_profileName;
    QLabel *m_profileDept;
    QLabel *m_profileSem;
    QLabel *m_profileSec;
    QLabel *m_profileSub;
    QLabel *m_profileEmptyMsg;

    // Tab 1 Action panel (inside visual tab)
    QComboBox   *m_studentCombo;
    QPushButton *m_assignBtn;
    QPushButton *m_unassignBtn;
    QPushButton *m_lockBtn;

    // Tab 2: Tabular Details
    QWidget      *m_listTab;
    QTableWidget *m_table;

    // Selection State
    int m_selectedBench = -1;
    int m_selectedSeat  = -1;

    // Helpers
    void buildUi();
    void buildVisualTab();
    void buildListTab();
    void populateStudentCombo();
    void updateProfilePanel(int bench, int seat);
    void refreshGrid();
    void refreshTable();
    QString benchSeatLabel(int bench, int seat) const;
    QColor statusBgColor(SeatStatus s) const;
    QString statusText(SeatStatus s) const;
};
