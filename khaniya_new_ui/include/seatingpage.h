#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QSplitter>
#include <QTableWidget>
#include "studentmodel.h"
#include "hallmap.h"

struct sqlite3;

class SeatingPage : public QWidget {
    Q_OBJECT
public:
    explicit SeatingPage(HallModel *model, sqlite3* db, QWidget *parent = nullptr);
    void refresh();
    void reloadRooms();
    void jumpToStudent(int studentId);

private slots:
    void onAutoAssign();
    void onClearAll();
    void onSubBlockClicked(const QString &block, int sub);
    void onSearch();
    void onSearchResultClicked(int row, int col);
    void onRoomSelected(int index);
    void toggleRoomInfo();
    void updateRoomInfo();

private:
    HallModel     *m_model;
    sqlite3       *m_db;
    HallMapWidget *m_hallMap;
    QLineEdit     *m_searchBox;
    QTableWidget  *m_searchResults;
    QLabel        *m_statusLbl;
    QComboBox     *m_roomSelector;
    QWidget       *m_roomInfoCard;
    QLabel        *m_roomInfoDetails;
    QPushButton   *m_toggleInfoBtn;

    void buildUi();
};
