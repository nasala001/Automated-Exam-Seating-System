#include "../include/exportmanager.h"
#include <QFile>
#include <QTextStream>
#include <QDateTime>

ExportManager::ExportManager(HallModel *model, QObject *parent)
    : QObject(parent), m_model(model) {}

QString ExportManager::htmlHeader() const {
    return R"(<!DOCTYPE html><html><head><meta charset="UTF-8">
<title>KU Examination Seating Chart</title>
<style>
  body { font-family: 'Segoe UI', Arial, sans-serif; color: #1a2332; background: #f5f7fa; margin: 0; padding: 20px; }
  h1 { color: #003366; border-bottom: 3px solid #c9a84c; padding-bottom: 8px; }
  h2 { color: #003366; font-size: 15px; margin-top: 24px; }
  table { border-collapse: collapse; width: 100%; margin-bottom: 16px; font-size: 12px; }
  th { background: #003366; color: white; padding: 8px 10px; text-align: left; }
  td { padding: 6px 10px; border-bottom: 1px solid #e0e7ef; }
  tr:nth-child(even) { background: #f8fafc; }
  .header-info { color: #6b7a8d; font-size: 12px; margin-bottom: 16px; }
  .badge { background: #003366; color: white; border-radius: 4px; padding: 2px 6px; font-size: 10px; }
  @media print { body { background: white; } }
</style></head><body>
<h1>Kathmandu University — Examination Seating Chart</h1>
<div class="header-info">Generated: )" + QDateTime::currentDateTime().toString("dd MMM yyyy hh:mm") + "</div>\n";
}

QString ExportManager::htmlFooter() const {
    return "</body></html>";
}

bool ExportManager::exportCSV(const QString &path) const {
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) return false;
    QTextStream ts(&f);
    ts << "Roll Number,Name,Department,Semester,Program,Subject,Block,SubBlock,Bench,Seat,Seat Code\n";
    for (Student *s : m_model->allStudents()) {
        ts << QString("%1,%2,%3,%4,%5,%6,%7,%8,%9,%10,%11\n")
              .arg(s->rollNumber).arg(s->name).arg(s->department)
              .arg(s->semester).arg(s->program).arg(s->subject)
              .arg(s->block).arg(s->subBlock).arg(s->bench).arg(s->seat)
              .arg(s->seatCode());
    }
    return true;
}

bool ExportManager::exportHTML(const QString &path) const {
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) return false;
    QTextStream ts(&f);
    ts << htmlHeader();

    // Summary
    ts << QString("<p><strong>Total Students:</strong> %1 &nbsp;|&nbsp; "
                  "<strong>Assigned:</strong> %2 &nbsp;|&nbsp; "
                  "<strong>Hall Capacity:</strong> %3 &nbsp;|&nbsp; "
                  "<strong>Utilization:</strong> %4%</p>\n")
          .arg(m_model->allStudents().size())
          .arg(m_model->occupiedCount())
          .arg(HallConst::CAPACITY)
          .arg((int)m_model->utilizationPercent());

    // Per-block tables
    for (const QString &block : HallConst::BLOCK_NAMES) {
        int bocc = m_model->occupiedInBlock(block);
        ts << QString("<h2>Block %1 &nbsp; <span class='badge'>%2 / %3</span></h2>\n")
              .arg(block).arg(bocc).arg(HallConst::PER_MAJOR);
        ts << "<table><tr><th>Sub-Block</th><th>Bench</th><th>Seat</th>"
              "<th>Roll No.</th><th>Name</th><th>Department</th><th>Semester</th><th>Status</th></tr>\n";
        for (int sub=1; sub<=3; sub++) {
            for (int bench=1; bench<=6; bench++) {
                for (int seat=1; seat<=2; seat++) {
                    const SeatCell &cell = m_model->seatAt(block, sub, bench, seat);
                    Student *s = (cell.studentId!=-1) ? m_model->findById(cell.studentId) : nullptr;
                    QString rowBg = s ? "#f0fff4" : "#ffffff";
                    ts << QString("<tr style='background:%1'><td>%2%3</td><td>%4</td><td>%5</td>"
                                  "<td>%6</td><td>%7</td><td>%8</td><td>%9</td><td>%10</td></tr>\n")
                          .arg(rowBg)
                          .arg(block).arg(sub)
                          .arg(bench)
                          .arg(seat==1?"Left":"Right")
                          .arg(s?s->rollNumber:"—")
                          .arg(s?s->name:"—")
                          .arg(s?s->department:"—")
                          .arg(s?QString("Sem %1").arg(s->semester):"—")
                          .arg(s?"Occupied":(cell.locked?"Locked":"Empty"));
                }
            }
        }
        ts << "</table>\n";
    }
    ts << htmlFooter();
    return true;
}

bool ExportManager::exportBlockReport(const QString &path) const {
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) return false;
    QTextStream ts(&f);
    ts << htmlHeader();
    ts << "<h2>Block Occupancy Report</h2>\n"
          "<table><tr><th>Block</th><th>Capacity</th><th>Occupied</th>"
          "<th>Available</th><th>Utilization %</th></tr>\n";
    for (const QString &b : HallConst::BLOCK_NAMES) {
        int occ = m_model->occupiedInBlock(b);
        int avail = HallConst::PER_MAJOR - occ;
        int pct   = (int)((double)occ/HallConst::PER_MAJOR*100);
        ts << QString("<tr><td><strong>%1</strong></td><td>%2</td><td>%3</td><td>%4</td><td>%5%</td></tr>\n")
              .arg(b).arg(HallConst::PER_MAJOR).arg(occ).arg(avail).arg(pct);
    }
    ts << QString("<tr style='background:#003366;color:white;font-weight:bold;'>"
                  "<td>TOTAL</td><td>%1</td><td>%2</td><td>%3</td><td>%4%</td></tr>\n")
          .arg(HallConst::CAPACITY).arg(m_model->occupiedCount())
          .arg(HallConst::CAPACITY - m_model->occupiedCount())
          .arg((int)m_model->utilizationPercent());
    ts << "</table>\n";
    ts << htmlFooter();
    return true;
}
