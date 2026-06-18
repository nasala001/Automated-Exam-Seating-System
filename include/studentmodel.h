#pragma once
#include <QString>
#include <QList>
#include <QMap>
#include <QObject>
#include <QJsonObject>
#include <QJsonArray>

// ── Seat Status ────────────────────────────────────────────────────────────
enum class SeatStatus {
    Empty,
    Assigned,
    Occupied,
    Reserved,
    Conflict
};

// ── Student Record ─────────────────────────────────────────────────────────
struct Student {
    int     id;
    QString name;
    QString rollNumber;
    QString department;
    int     semester;
    QString program;    // BE, BArch, BSc, etc.
    QString subject;

    // Assigned seat info
    QString block;      // A-I
    int     subBlock;   // 1-3
    int     bench;      // 1-6
    int     seat;       // 1 or 2  (L=1, R=2)
    SeatStatus status = SeatStatus::Empty;

    bool isAssigned() const { return !block.isEmpty(); }
    QString seatCode() const {
        if (!isAssigned()) return "--";
        return QString("%1%2-B%3-S%4").arg(block).arg(subBlock).arg(bench).arg(seat);
    }
};

// ── Seat Cell ───────────────────────────────────────────────────────────────
struct SeatCell {
    int     studentId = -1;   // -1 = empty
    SeatStatus status = SeatStatus::Empty;
    bool    locked    = false;
};

// ── Hall Layout Constants ───────────────────────────────────────────────────
namespace HallConst {
    constexpr int MAJOR_BLOCKS   = 9;
    constexpr int SUB_BLOCKS     = 3;
    constexpr int BENCHES        = 6;
    constexpr int SEATS_PER_BENCH= 2;
    constexpr int CAPACITY       = MAJOR_BLOCKS * SUB_BLOCKS * BENCHES * SEATS_PER_BENCH; // 324
    constexpr int PER_MAJOR      = SUB_BLOCKS * BENCHES * SEATS_PER_BENCH;   // 36
    constexpr int PER_SUB        = BENCHES * SEATS_PER_BENCH;                // 12

    const QStringList BLOCK_NAMES = {"A","B","C","D","E","F","G","H","I"};
    // Row × Col layout
    // Row 0: A B C
    // Row 1: D E F
    // Row 2: G H I
}

// ── Hall Model ─────────────────────────────────────────────────────────────
class HallModel : public QObject {
    Q_OBJECT
public:
    explicit HallModel(QObject *parent = nullptr);

    // Student CRUD
    void        addStudent(const Student &s);
    void        updateStudent(const Student &s);
    void        removeStudent(int id);
    Student*    findById(int id);
    Student*    findByRoll(const QString &roll);
    QList<Student*> searchStudents(const QString &query) const;
    QList<Student*> allStudents();
    QList<Student*> studentsByBlock(const QString &block);
    QList<Student*> studentsByDept(const QString &dept);
    QList<Student*> unassignedStudents();

    // Seat access  block="A"..I, sub=1..3, bench=1..6, seat=1..2
    SeatCell&   seatAt(const QString &block, int sub, int bench, int seat);
    const SeatCell& seatAt(const QString &block, int sub, int bench, int seat) const;

    // Assignment
    bool        assignSeat(int studentId, const QString &block, int sub, int bench, int seat);
    bool        unassignSeat(const QString &block, int sub, int bench, int seat);
    void        clearAllAssignments();
    bool        autoAssign();           // sequential fill
    bool        lockSeat(const QString &block, int sub, int bench, int seat, bool lock);

    // Capacity queries
    int         occupiedCount() const;
    int         occupiedInBlock(const QString &block) const;
    int         occupiedInSubBlock(const QString &block, int sub) const;
    double      utilizationPercent() const;

    // Departments & semesters
    QStringList departments() const;
    QList<int>  semesters() const;

    // Conflict detection
    QList<QPair<int,int>> detectConflicts();  // returns pairs of conflicting student ids

    // Import / Export helpers
    void        loadSampleData();
    QJsonObject toJson() const;
    void        fromJson(const QJsonObject &obj);

signals:
    void dataChanged();
    void seatAssigned(int studentId, const QString &seatCode);
    void conflictDetected(const QString &msg);

private:
    int  nextId = 1;
    QMap<int, Student> m_students;  // id → student

    // Hall seats: [blockIdx 0-8][subIdx 0-2][benchIdx 0-5][seatIdx 0-1]
    SeatCell m_seats[9][3][6][2];

    int blockIndex(const QString &block) const;
};
