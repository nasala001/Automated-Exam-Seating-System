#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include "../../nasala_hall_room/include/HallRepository.h"
#include "../../nasala_hall_room/include/RoomRepository.h"
#include "../../nasala_hall_room/include/HallManager.h"

class RoomManagerPage : public QWidget {
    Q_OBJECT
public:
    explicit RoomManagerPage(sqlite3* db, QWidget *parent = nullptr);
    ~RoomManagerPage();

    void refresh();

signals:
    void roomChanged();

private slots:
    void onAddHall();
    void onRemoveHall();
    void onAddRoom();
    void onRemoveRoom();
    void onHallSelected(int row, int col);

private:
    sqlite3* m_db = nullptr;
    bool m_firstLoad = false;
    nasala::HallManager m_manager;

    QTableWidget* m_hallTable = nullptr;
    QTableWidget* m_roomTable = nullptr;
    QLabel* m_selectedHallLbl = nullptr;

    void buildUi();
    void loadData();
    void populateHalls(const QString& selectHallID = "");
    void populateRooms(const QString& hallID, const QString& selectRoomID = "");
    QString selectedHallID() const;
    QString selectedRoomID() const;
};
