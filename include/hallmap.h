#pragma once
#include <QWidget>
#include <QMap>
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

// ── Hall Map Widget ─────────────────────────────────────────────────────────
// Full hall overview: 3×3 grid of major blocks
class HallMapWidget : public QWidget {
    Q_OBJECT
public:
    explicit HallMapWidget(HallModel *model, QWidget *parent=nullptr);
    void refresh();
    void highlightStudent(int studentId);  // scroll to & highlight a student's seat

signals:
    void subBlockClicked(const QString &block, int sub);

private:
    HallModel *m_model;
    QList<MajorBlockWidget*> m_blocks;
    int m_highlightStudentId = -1;
};
