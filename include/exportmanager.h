#pragma once
#include <QObject>
#include <QString>
#include "studentmodel.h"

class ExportManager : public QObject {
    Q_OBJECT
public:
    explicit ExportManager(HallModel *model, QObject *parent=nullptr);

    bool exportCSV(const QString &path) const;
    bool exportHTML(const QString &path) const;  // printable seating chart
    bool exportBlockReport(const QString &path) const;

private:
    HallModel *m_model;
    QString htmlHeader() const;
    QString htmlFooter() const;
};
