#pragma once
#include "../../shresh_validation/DataStructures.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QList>
#include <QMap>
#include <QObject>
#include <QString>
#include <SeatCodeUtils.h>


//  Seat Status
enum class SeatStatus { Empty, Assigned, Occupied, Reserved, Conflict };

//  UIStudent Record
struct UIStudent {
  int id;
  QString name;
  QString rollNumber;
  QString department;
  int semester;
  QString program; // BE, BArch, BSc, etc.
  QString section; // A, B, etc.
  QString subject;

  // Assigned seat info
  QString roomCode;  // Room ID
  QString venueCode; // Short/normalized venue code (e.g. "MPH")
  QString block;     // A-I (empty for custom rooms)
  int subBlock = 0;  // 1-3
  int bench = 0;     // 1-6
  int seat = 0;      // 1 or 2
  int row = -1;      // -1 = unassigned, 0-indexed row
  int col = -1;      // -1 = unassigned, 0-indexed col
  SeatStatus status = SeatStatus::Empty;

  // Additional fields for engine compatibility
  QString registrationNo;
  bool isPhysicallyImpaired = false;

  bool isAssigned() const { return row != -1; }
  QString seatCode() const {
    if (!isAssigned())
      return "--";
    if (!block.isEmpty()) {
      return QString("%1%2-B%3-S%4")
          .arg(block)
          .arg(subBlock)
          .arg(bench)
          .arg(seat);
    }
    QString rowLetter = "";
    int r = row;
    while (r >= 0) {
      rowLetter = QChar('A' + (r % 26)) + rowLetter;
      r = (r / 26) - 1;
    }
    QString vCode = venueCode.isEmpty() ? roomCode : venueCode;
    return QString("%1-%2-%3").arg(vCode).arg(rowLetter).arg(col + 1);
  }
};

//  Seat Cell
struct SeatCell {
  int studentId = -1; // -1 = empty
  SeatStatus status = SeatStatus::Empty;
  bool locked = false;
};

//  Hall Layout Constants
namespace HallConst {
constexpr int MAJOR_BLOCKS = 9;
constexpr int SUB_BLOCKS = 3;
constexpr int BENCHES = 6;
constexpr int SEATS_PER_BENCH = 2;
constexpr int CAPACITY =
    MAJOR_BLOCKS * SUB_BLOCKS * BENCHES * SEATS_PER_BENCH;        // 324
constexpr int PER_MAJOR = SUB_BLOCKS * BENCHES * SEATS_PER_BENCH; // 36
constexpr int PER_SUB = BENCHES * SEATS_PER_BENCH;                // 12

const QStringList BLOCK_NAMES = {"A", "B", "C", "D", "E", "F", "G", "H", "I"};
// Row x Col layout
// Row 0: A B C
// Row 1: D E F
// Row 2: G H I
} // namespace HallConst

//  Hall Model
class HallModel : public QObject {
  Q_OBJECT
public:
  explicit HallModel(QObject *parent = nullptr);

  // UIStudent CRUD
  // UIStudent CRUD
  void addStudent(const UIStudent &s);
  void updateStudent(const UIStudent &s);
  void removeStudent(int id);
  UIStudent *findById(int id);
  UIStudent *findByRoll(const QString &roll);
  QList<UIStudent *> searchStudents(const QString &query) const;
  QList<UIStudent *> allStudents();
  QList<UIStudent *> studentsByBlock(const QString &block);
  QList<UIStudent *> studentsByDept(const QString &dept);
  QList<UIStudent *> unassignedStudents();

  // Seat access  block="A"..I, sub=1..3, bench=1..6, seat=1..2
  SeatCell &seatAt(const QString &block, int sub, int bench, int seat);
  const SeatCell &seatAt(const QString &block, int sub, int bench,
                         int seat) const;
  SeatCell &seatAt(int row, int col);
  const SeatCell &seatAt(int row, int col) const;

  // Seat Naming
  QString getSeatName(int row, int col) const;

  // Assignment
  bool assignSeat(int studentId, const QString &block, int sub, int bench,
                  int seat);
  bool unassignSeat(const QString &block, int sub, int bench, int seat);
  bool assignSeat(int studentId, int row, int col);
  bool unassignSeat(int row, int col);
  void clearAllAssignments();
  void clearAllStudents();
  bool autoAssign(); // sequential fill
  bool lockSeat(const QString &block, int sub, int bench, int seat, bool lock);
  bool lockSeat(int row, int col, bool lock);

  // Active Room Config
  void setActiveRoom(const QString &hallID, const QString &roomID, int rows,
                     int cols, const QString &venueName = "");
  QString activeRoomID() const { return m_activeRoomID; }
  QString activeHallID() const { return m_activeHallID; }
  int rows() const { return m_rows; }
  int cols() const { return m_cols; }

  // Capacity queries
  int occupiedCount() const;
  int occupiedInBlock(const QString &block) const;
  int occupiedInSubBlock(const QString &block, int sub) const;
  double utilizationPercent() const;

  // Departments & semesters
  QStringList departments() const;
  QList<int> semesters() const;

  // Helper
  int blockIndex(const QString &block) const;

  // Conflict detection
  QList<QPair<int, int>>
  detectConflicts(); // returns pairs of conflicting UIStudent ids

  // Import / Export helpers
  void loadSampleData();
  QJsonObject toJson() const;
  void fromJson(const QJsonObject &obj);
  void saveToCSV(const QString &path) const;
  void loadFromCSV(const QString &path);

signals:
  void dataChanged();
  void seatAssigned(int studentId, const QString &seatCode);
  void conflictDetected(const QString &msg);

private:
  int nextId = 1;
  QMap<int, UIStudent> m_students; // id  UIStudent

  QString m_activeHallID = "MAIN_HALL";
  QString m_activeRoomID = "MAIN_ROOM";
  QString m_activeVenueCode;
  int m_rows = 18;
  int m_cols = 18;
  std::vector<std::vector<SeatCell>> m_seatsGrid;
};
