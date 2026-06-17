#pragma once
#include <QWidget>
#include <QLabel>
#include <QTableWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QLineEdit>
#include <QSpinBox>

class MainWindow;

class RoomPage : public QWidget
{
    Q_OBJECT

public:
    explicit RoomPage(MainWindow *mainWin, QWidget *parent = nullptr);

signals:
    void venueAdded();

private slots:
    // Hall tab
    void addSection();
    void removeSection();
    void saveHall();

    // Room tab
    void addRoom();

    // Venue summary
    void deleteVenue();

private:
    MainWindow *mainWindow;

    // ── Hall tab widgets ───────────────
    QLineEdit    *hallNameEdit;
    QSpinBox     *hallRowsSpin;
    QTableWidget *sectionTable;

    // ── Room tab widgets ───────────────
    QLineEdit    *blockNameEdit;
    QLineEdit    *roomNumberEdit;
    QSpinBox     *roomRowsSpin;
    QSpinBox     *roomColsSpin;

    // ── Venue summary ──────────────────
    QTableWidget *venueSummaryTable;

    QWidget* createHallTab();
    QWidget* createRoomTab();
    void refreshVenueSummary();
};