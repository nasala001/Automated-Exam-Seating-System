#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QSplitter>
#include <QTableWidget>
#include "studentmodel.h"
#include "hallmap.h"

class SeatingPage : public QWidget {
    Q_OBJECT
public:
    explicit SeatingPage(HallModel *model, QWidget *parent=nullptr);
    void refresh();
    void jumpToStudent(int studentId);

private slots:
    void onAutoAssign();
    void onClearAll();
    void onSubBlockClicked(const QString &block, int sub);
    void onSearch();
    void onSearchResultClicked(int row, int col);

private:
    HallModel     *m_model;
    HallMapWidget *m_hallMap;
    QLineEdit     *m_searchBox;
    QTableWidget  *m_searchResults;
    QLabel        *m_statusLbl;

    void buildUi();
};
