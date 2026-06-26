#pragma once
#include <QWidget>
#include <QMap>
#include <QVBoxLayout>
#include <QtGlobal>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QEnterEvent>
#endif
#include "studentmodel.h"

// ── SubBlock Mini-Cell Widget ───────────────────────────────────────────────
// Small clickable tile representing one sub-block in the hall map
class SubBlockTile : public QWidget {
    Q_OBJECT
public:
    SubBlockTile(const QString &block, int sub, HallModel *model, QWidget *parent=nullptr);
    void refresh();

signals:
    void clicked(const QString &block, int sub);

protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent *) override;
#else
    void enterEvent(QEvent *) override;
#endif
    void leaveEvent(QEvent *) override;

private:
    QString    m_block;
    int        m_sub;
    HallModel *m_model;
    bool       m_hovered = false;
};

// ── Major Block Widget ──────────────────────────────────────────────────────
// Represents one of the 9 major blocks (A-I) with its 3 sub-blocks
class MajorBlockWidget : public QWidget {
    Q_OBJECT
public:
    MajorBlockWidget(const QString &block, HallModel *model, QWidget *parent=nullptr);
    void refresh();

signals:
    void subBlockClicked(const QString &block, int sub);

protected:
    void paintEvent(QPaintEvent *) override;

private:
    QString    m_block;
    HallModel *m_model;
    QList<SubBlockTile*> m_tiles;
};

// ── Single Seat Cell Widget (for custom room rendering) ─────────────────────
class SeatCellWidget : public QWidget {
    Q_OBJECT
public:
    SeatCellWidget(int row, int col, HallModel *model, QWidget *parent=nullptr);
    void refresh();

signals:
    void clicked(int row, int col);

protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent *) override;
#else
    void enterEvent(QEvent *) override;
#endif
    void leaveEvent(QEvent *) override;

private:
    int m_row;
    int m_col;
    HallModel *m_model;
    bool m_hovered = false;
};

// ── Hall Map Widget ─────────────────────────────────────────────────────────
// Full hall overview: 3×3 grid of major blocks, or direct rows x cols grid
class HallMapWidget : public QWidget {
    Q_OBJECT
public:
    explicit HallMapWidget(HallModel *model, QWidget *parent=nullptr);
    void refresh();
    void highlightStudent(int studentId);  // scroll to & highlight a UIStudent's seat

signals:
    void subBlockClicked(const QString &block, int sub);
    void seatClicked(int row, int col);

private slots:
    void onSeatClicked(int row, int col);

private:
    HallModel *m_model;
    QList<MajorBlockWidget*> m_blocks;
    QList<SeatCellWidget*> m_seats;
    QWidget *m_gridContainer = nullptr;
    QVBoxLayout *m_mainVL = nullptr;
    int m_highlightstudentId = -1;
};
