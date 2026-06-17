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
#include <QComboBox>

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
    void previewHall();

    // Room tab
    void addRoom();

    // Venue summary
    void deleteVenue();
    void visualizeVenue(int row);

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

    // ── Seat layout visualization ──────
    QTableWidget *seatGridPreview;
    QLabel       *previewTitle;

    QWidget* createHallTab();
    QWidget* createRoomTab();
    void refreshVenueSummary();
    void showSeatGrid(int venueIndex);
};