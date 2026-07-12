#include "../include/studentmodel.h"
#include "../../include/io/DatabaseManager.h"
using namespace shresh;
#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QTextStream>
#include <SeatCodeUtils.h>
#include <algorithm>
#include <random>


HallModel::HallModel(QObject *parent) : QObject(parent) {
  m_seatsGrid.resize(m_rows, std::vector<SeatCell>(m_cols));
}

int HallModel::blockIndex(const QString &block) const {
  return HallConst::BLOCK_NAMES.indexOf(block.toUpper());
}

// ── CRUD ────────────────────────────────────────────────────────────────────
void HallModel::addStudent(const UIStudent &s) {
  UIStudent ns = s;
  if (ns.id <= 0)
    ns.id = nextId++;
  else if (ns.id >= nextId)
    nextId = ns.id + 1;
  m_students[ns.id] = ns;
  emit dataChanged();
}

void HallModel::updateStudent(const UIStudent &s) {
  if (m_students.contains(s.id)) {
    m_students[s.id] = s;
    emit dataChanged();
  }
}

void HallModel::removeStudent(int id) {
  if (!m_students.contains(id))
    return;
  UIStudent &st = m_students[id];
  if (st.isAssigned()) {
    unassignSeat(st.row, st.col);
  }
  m_students.remove(id);
  emit dataChanged();
}

UIStudent *HallModel::findById(int id) {
  auto it = m_students.find(id);
  return (it != m_students.end()) ? &it.value() : nullptr;
}

UIStudent *HallModel::findByRoll(const QString &roll) {
  for (auto &s : m_students)
    if (s.rollNumber.compare(roll, Qt::CaseInsensitive) == 0)
      return &s;
  return nullptr;
}

QList<UIStudent *> HallModel::searchStudents(const QString &q) const {
  QList<UIStudent *> results;
  QString ql = q.toLower();
  for (auto &s : m_students) {
    if (s.name.toLower().contains(ql) || s.rollNumber.toLower().contains(ql) ||
        s.department.toLower().contains(ql))
      results.append(const_cast<UIStudent *>(&s));
  }
  return results;
}

QList<UIStudent *> HallModel::allStudents() {
  QList<UIStudent *> list;
  for (auto &s : m_students)
    list.append(&s);
  return list;
}

QList<UIStudent *> HallModel::studentsByBlock(const QString &block) {
  QList<UIStudent *> list;
  for (auto &s : m_students)
    if (s.block == block)
      list.append(&s);
  return list;
}

QList<UIStudent *> HallModel::studentsByDept(const QString &dept) {
  QList<UIStudent *> list;
  for (auto &s : m_students)
    if (s.department == dept)
      list.append(&s);
  return list;
}

QList<UIStudent *> HallModel::unassignedStudents() {
  QList<UIStudent *> list;
  for (auto &s : m_students)
    if (!s.isAssigned())
      list.append(&s);
  return list;
}

// ── Seat Access ─────────────────────────────────────────────────────────────
SeatCell &HallModel::seatAt(int row, int col) {
  if (row < 0 || row >= m_rows || col < 0 || col >= m_cols) {
    static SeatCell dummy;
    return dummy;
  }
  return m_seatsGrid[row][col];
}

const SeatCell &HallModel::seatAt(int row, int col) const {
  if (row < 0 || row >= m_rows || col < 0 || col >= m_cols) {
    static SeatCell dummy;
    return dummy;
  }
  return m_seatsGrid[row][col];
}

SeatCell &HallModel::seatAt(const QString &block, int sub, int bench,
                            int seat) {
  int bi = blockIndex(block);
  if (bi < 0 || sub < 1 || sub > 3 || bench < 1 || bench > 6 || seat < 1 ||
      seat > 2) {
    static SeatCell dummy;
    return dummy;
  }
  int r = (bi / 3) * 6 + (bench - 1);
  int c = (bi % 3) * 6 + (sub - 1) * 2 + (seat - 1);
  return seatAt(r, c);
}

const SeatCell &HallModel::seatAt(const QString &block, int sub, int bench,
                                  int seat) const {
  int bi = blockIndex(block);
  if (bi < 0 || sub < 1 || sub > 3 || bench < 1 || bench > 6 || seat < 1 ||
      seat > 2) {
    static SeatCell dummy;
    return dummy;
  }
  int r = (bi / 3) * 6 + (bench - 1);
  int c = (bi % 3) * 6 + (sub - 1) * 2 + (seat - 1);
  return seatAt(r, c);
}

QString HallModel::getSeatName(int row, int col) const {
  if (m_activeRoomID == "MAIN_ROOM") {
    int bi = (row / 6) * 3 + (col / 6);
    int bench = (row % 6) + 1;
    int sub = ((col % 6) / 2) + 1;
    int seat = (col % 2) + 1;
    if (bi >= 0 && bi < 9) {
      QString block = HallConst::BLOCK_NAMES[bi];
      return QString("%1%2-B%3-S%4").arg(block).arg(sub).arg(bench).arg(seat);
    }
  }

  // Custom room format
  QString rowLetter = "";
  int r = row;
  while (r >= 0) {
    rowLetter = QChar('A' + (r % 26)) + rowLetter;
    r = (r / 26) - 1;
  }
  QString vCode =
      m_activeVenueCode.isEmpty() ? m_activeRoomID : m_activeVenueCode;
  return QString("%1-%2-%3").arg(vCode).arg(rowLetter).arg(col + 1);
}

// ── Assignment ───────────────────────────────────────────────────────────────
bool HallModel::assignSeat(int studentId, const QString &block, int sub,
                           int bench, int seat) {
  int bi = blockIndex(block);
  if (bi < 0)
    return false;
  int r = (bi / 3) * 6 + (bench - 1);
  int c = (bi % 3) * 6 + (sub - 1) * 2 + (seat - 1);

  UIStudent *st = findById(studentId);
  if (!st)
    return false;

  if (assignSeat(studentId, r, c)) {
    st->block = block;
    st->subBlock = sub;
    st->bench = bench;
    st->seat = seat;
    emit seatAssigned(studentId, st->seatCode());
    return true;
  }
  return false;
}

bool HallModel::assignSeat(int studentId, int row, int col) {
  UIStudent *st = findById(studentId);
  if (!st)
    return false;
  if (row < 0 || row >= m_rows || col < 0 || col >= m_cols)
    return false;
  SeatCell &cell = m_seatsGrid[row][col];
  if (cell.locked)
    return false;
  if (cell.studentId != -1 && cell.studentId != studentId)
    return false;

  if (st->isAssigned()) {
    unassignSeat(st->row, st->col);
  }

  cell.studentId = studentId;
  cell.status = SeatStatus::Occupied;

  st->roomCode = m_activeRoomID;
  st->venueCode = m_activeVenueCode;
  st->block.clear();
  st->subBlock = 0;
  st->bench = 0;
  st->seat = 0;
  st->row = row;
  st->col = col;
  st->status = SeatStatus::Occupied;

  emit seatAssigned(studentId, st->seatCode());
  emit dataChanged();
  return true;
}

bool HallModel::unassignSeat(const QString &block, int sub, int bench,
                             int seat) {
  int bi = blockIndex(block);
  if (bi < 0)
    return false;
  int r = (bi / 3) * 6 + (bench - 1);
  int c = (bi % 3) * 6 + (sub - 1) * 2 + (seat - 1);
  return unassignSeat(r, c);
}

bool HallModel::unassignSeat(int row, int col) {
  if (row < 0 || row >= m_rows || col < 0 || col >= m_cols)
    return false;
  SeatCell &cell = m_seatsGrid[row][col];
  if (cell.locked)
    return false;
  if (cell.studentId != -1) {
    UIStudent *st = findById(cell.studentId);
    if (st) {
      st->block.clear();
      st->subBlock = 0;
      st->bench = 0;
      st->seat = 0;
      st->row = -1;
      st->col = -1;
      st->status = SeatStatus::Empty;
    }
  }
  cell = SeatCell{};
  emit dataChanged();
  return true;
}

void HallModel::clearAllAssignments() {
  for (int r = 0; r < m_rows; ++r) {
    for (int c = 0; c < m_cols; ++c) {
      if (!m_seatsGrid[r][c].locked) {
        m_seatsGrid[r][c] = SeatCell{};
      }
    }
  }
  for (auto &s : m_students) {
    s.block.clear();
    s.subBlock = 0;
    s.bench = 0;
    s.seat = 0;
    s.row = -1;
    s.col = -1;
    s.status = SeatStatus::Empty;
  }
  emit dataChanged();
}

void HallModel::clearAllStudents() {
  clearAllAssignments(); // safely remove from grid first
  m_students.clear();
  nextId = 1;
  emit dataChanged();
}

bool HallModel::autoAssign() {
  bool oldState = blockSignals(true);
  QList<UIStudent *> unassigned = unassignedStudents();
  if (unassigned.isEmpty()) {
    blockSignals(oldState);
    return true;
  }

  // ── Group students by department for round-robin interleaving ──
  QMap<QString, QList<UIStudent *>> deptGroups;
  for (UIStudent *s : unassigned) {
    deptGroups[s->department].append(s);
  }

  // Build ordered buckets, largest department first for best spread
  QList<QList<UIStudent *>> buckets;
  QList<QString> deptKeys = deptGroups.keys();
  std::sort(deptKeys.begin(), deptKeys.end(),
            [&](const QString &a, const QString &b) {
              return deptGroups[a].size() > deptGroups[b].size();
            });
  for (const QString &key : deptKeys) {
    buckets.append(deptGroups[key]);
  }

  if (buckets.isEmpty()) {
    blockSignals(oldState);
    return true;
  }

  // Round-robin indices for each bucket
  QList<int> indices(buckets.size(), 0);
  int currentBucket = 0;
  int placed = 0;

  for (int r = 0; r < m_rows; ++r) {
    // Offset starting bucket per row to prevent vertical alignment
    int rowStartBucket = (buckets.size() > 1 && m_cols % buckets.size() == 0)
                             ? (r % buckets.size())
                             : currentBucket;
    currentBucket = rowStartBucket;

    for (int c = 0; c < m_cols; ++c) {
      SeatCell &cell = m_seatsGrid[r][c];
      if (cell.studentId != -1 || cell.locked)
        continue;

      // Find the next student from a different department than the left
      // neighbor
      QString leftDept;
      if (c > 0 && m_seatsGrid[r][c - 1].studentId != -1) {
        UIStudent *leftSt = findById(m_seatsGrid[r][c - 1].studentId);
        if (leftSt)
          leftDept = leftSt->department;
      }

      bool foundStudent = false;
      for (int attempt = 0; attempt < buckets.size(); ++attempt) {
        int bIdx = (currentBucket + attempt) % buckets.size();
        if (indices[bIdx] < buckets[bIdx].size()) {
          UIStudent *candidate = buckets[bIdx][indices[bIdx]];
          // Skip if same department as left neighbor (try next bucket)
          if (!leftDept.isEmpty() && candidate->department == leftDept &&
              attempt + 1 < buckets.size()) {
            continue;
          }
          // Place this student
          indices[bIdx]++;
          if (m_activeRoomID == "MAIN_ROOM") {
            int bi = (r / 6) * 3 + (c / 6);
            int bench = (r % 6) + 1;
            int sub = (c % 6) / 2 + 1;
            int seat = (c % 2) + 1;
            assignSeat(candidate->id, HallConst::BLOCK_NAMES[bi], sub, bench,
                       seat);
          } else {
            assignSeat(candidate->id, r, c);
          }
          currentBucket = (bIdx + 1) % buckets.size();
          placed++;
          foundStudent = true;
          break;
        }
      }

      // If we couldn't avoid same-dept, just place any remaining student
      if (!foundStudent) {
        for (int bIdx = 0; bIdx < buckets.size(); ++bIdx) {
          if (indices[bIdx] < buckets[bIdx].size()) {
            UIStudent *candidate = buckets[bIdx][indices[bIdx]];
            indices[bIdx]++;
            if (m_activeRoomID == "MAIN_ROOM") {
              int bi = (r / 6) * 3 + (c / 6);
              int bench = (r % 6) + 1;
              int sub = (c % 6) / 2 + 1;
              int seat = (c % 2) + 1;
              assignSeat(candidate->id, HallConst::BLOCK_NAMES[bi], sub, bench,
                         seat);
            } else {
              assignSeat(candidate->id, r, c);
            }
            currentBucket = (bIdx + 1) % buckets.size();
            placed++;
            break;
          }
        }
      }

      // Check if all students are placed
      bool allPlaced = true;
      for (int b = 0; b < buckets.size(); ++b) {
        if (indices[b] < buckets[b].size()) {
          allPlaced = false;
          break;
        }
      }
      if (allPlaced)
        goto done;
    }
  }
done:
  blockSignals(oldState);
  emit dataChanged();
  return (placed == unassigned.size());
}

bool HallModel::lockSeat(const QString &block, int sub, int bench, int seat,
                         bool lock) {
  int bi = blockIndex(block);
  if (bi < 0)
    return false;
  int r = (bi / 3) * 6 + (bench - 1);
  int c = (bi % 3) * 6 + (sub - 1) * 2 + (seat - 1);
  return lockSeat(r, c, lock);
}

bool HallModel::lockSeat(int row, int col, bool lock) {
  if (row < 0 || row >= m_rows || col < 0 || col >= m_cols)
    return false;
  SeatCell &cell = m_seatsGrid[row][col];
  if (lock && cell.studentId != -1)
    return false;
  cell.locked = lock;
  emit dataChanged();
  return true;
}

void HallModel::setActiveRoom(const QString &hallID, const QString &roomID,
                              int rows, int cols, const QString &venueName) {
  m_activeHallID = hallID;
  m_activeRoomID = roomID;
  m_rows = rows;
  m_cols = cols;
  m_seatsGrid.clear();
  m_seatsGrid.resize(rows, std::vector<SeatCell>(cols));

  QString nameToNormalize = venueName;
  if (nameToNormalize.isEmpty()) {
    nameToNormalize = roomID;
  }
  m_activeVenueCode =
      QString::fromStdString(normalizeVenueName(nameToNormalize.toStdString()));

  // Reset student row/col properties
  for (auto &s : m_students) {
    s.block.clear();
    s.subBlock = 0;
    s.bench = 0;
    s.seat = 0;
    s.row = -1;
    s.col = -1;
    s.status = SeatStatus::Empty;
    s.venueCode.clear();
  }
  emit dataChanged();
}

// ── Capacity ─────────────────────────────────────────────────────────────────
int HallModel::occupiedCount() const {
  int c = 0;
  for (int r = 0; r < m_rows; ++r)
    for (int col = 0; col < m_cols; ++col)
      if (m_seatsGrid[r][col].studentId != -1)
        c++;
  return c;
}

int HallModel::occupiedInBlock(const QString &block) const {
  int bi = blockIndex(block);
  if (bi < 0 || m_activeRoomID != "MAIN_ROOM")
    return 0;
  int c = 0;
  int rStart = (bi / 3) * 6;
  int cStart = (bi % 3) * 6;
  for (int r = rStart; r < rStart + 6; ++r)
    for (int col = cStart; col < cStart + 6; ++col)
      if (m_seatsGrid[r][col].studentId != -1)
        c++;
  return c;
}

int HallModel::occupiedInSubBlock(const QString &block, int sub) const {
  int bi = blockIndex(block);
  if (bi < 0 || m_activeRoomID != "MAIN_ROOM")
    return 0;
  int c = 0;
  int rStart = (bi / 3) * 6;
  int cStart = (bi % 3) * 6 + (sub - 1) * 2;
  for (int r = rStart; r < rStart + 6; ++r)
    for (int col = cStart; col < cStart + 2; ++col)
      if (m_seatsGrid[r][col].studentId != -1)
        c++;
  return c;
}

double HallModel::utilizationPercent() const {
  int cap = m_rows * m_cols;
  if (cap == 0)
    return 0.0;
  return (double)occupiedCount() / cap * 100.0;
}

QStringList HallModel::departments() const {
  QStringList depts;
  for (auto &s : m_students)
    if (!depts.contains(s.department))
      depts.append(s.department);
  depts.sort();
  return depts;
}

QList<int> HallModel::semesters() const {
  QList<int> sems;
  for (auto &s : m_students)
    if (!sems.contains(s.semester))
      sems.append(s.semester);
  std::sort(sems.begin(), sems.end());
  return sems;
}

QList<QPair<int, int>> HallModel::detectConflicts() {
  QList<QPair<int, int>> conflicts;
  // Simple: same dept same semester adjacent seats
  // For now just mark duplicates
  QMap<QString, int> seatMap;
  for (auto &s : m_students) {
    if (!s.isAssigned())
      continue;
    QString key = s.seatCode();
    if (seatMap.contains(key)) {
      conflicts.append({seatMap[key], s.id});
      s.status = SeatStatus::Conflict;
    } else {
      seatMap[key] = s.id;
    }
  }
  return conflicts;
}

// ── Sample Data ──────────────────────────────────────────────────────────────
void HallModel::loadSampleData() {
  QStringList paths = {"data/students.csv", "../data/students.csv",
                       "../../data/students.csv"};

  std::vector<Student> students;
  for (const QString &p : paths) {
    if (QFile::exists(p)) {
      students = DatabaseManager::loadStudentsFromCSV(p.toStdString());
      if (!students.empty())
        break;
    }
  }

  if (students.empty()) {
    qWarning()
        << "Could not load sample data: data/students.csv not found or empty.";
    return;
  }

  for (const auto &s : students) {
    UIStudent newStudent;
    newStudent.id = nextId++;
    newStudent.name = QString::fromStdString(s.name);
    newStudent.rollNumber = QString::fromStdString(s.rollNo);
    newStudent.department = QString::fromStdString(s.department);
    newStudent.semester = s.semester;
    newStudent.program = QString::fromStdString(s.program);
    newStudent.subject = QString::fromStdString(s.subject);
    newStudent.status = SeatStatus::Empty;
    newStudent.registrationNo = QString::fromStdString(s.registrationNo);
    newStudent.isPhysicallyImpaired = s.isPhysicallyImpaired;
    m_students[newStudent.id] = newStudent;
  }
  emit dataChanged();
}

QJsonObject HallModel::toJson() const {
  QJsonObject root;
  root["hallID"] = m_activeHallID;
  root["roomID"] = m_activeRoomID;
  root["rows"] = m_rows;
  root["cols"] = m_cols;
  QJsonArray students;
  for (auto &s : m_students) {
    QJsonObject obj;
    obj["id"] = s.id;
    obj["name"] = s.name;
    obj["roll"] = s.rollNumber;
    obj["dept"] = s.department;
    obj["semester"] = s.semester;
    obj["program"] = s.program;
    obj["section"] = s.section;
    obj["subject"] = s.subject;
    obj["block"] = s.block;
    obj["subBlock"] = s.subBlock;
    obj["bench"] = s.bench;
    obj["seat"] = s.seat;
    obj["row"] = s.row;
    obj["col"] = s.col;
    obj["roomCode"] = s.roomCode;
    students.append(obj);
  }
  root["students"] = students;
  return root;
}

void HallModel::fromJson(const QJsonObject &obj) {
  m_students.clear();
  m_activeHallID = obj["hallID"].toString("MAIN_HALL");
  m_activeRoomID = obj["roomID"].toString("MAIN_ROOM");
  m_rows = obj["rows"].toInt(18);
  m_cols = obj["cols"].toInt(18);

  m_seatsGrid.clear();
  m_seatsGrid.resize(m_rows, std::vector<SeatCell>(m_cols));
  nextId = 1;
  QJsonArray students = obj["students"].toArray();
  for (auto v : students) {
    QJsonObject o = v.toObject();
    UIStudent s;
    s.id = o["id"].toInt();
    s.name = o["name"].toString();
    s.rollNumber = o["roll"].toString();
    s.department = o["dept"].toString();
    s.semester = o["semester"].toInt();
    s.program = o["program"].toString();
    s.section = o["section"].toString();
    s.subject = o["subject"].toString();
    s.block = o["block"].toString();
    s.subBlock = o["subBlock"].toInt();
    s.bench = o["bench"].toInt();
    s.seat = o["seat"].toInt();
    s.row = o["row"].toInt(-1);
    s.col = o["col"].toInt(-1);
    s.roomCode = o["roomCode"].toString();
    // Fallback for old save files that didn't have row/col
    if (s.row == -1 && !s.block.isEmpty()) {
      int bi = blockIndex(s.block);
      if (bi >= 0) {
        s.row = (bi / 3) * 6 + (s.bench - 1);
        s.col = (bi % 3) * 6 + (s.subBlock - 1) * 2 + (s.seat - 1);
      }
    }
    if (s.isAssigned()) {
      s.status = SeatStatus::Occupied;
      SeatCell &cell = seatAt(s.row, s.col);
      cell.studentId = s.id;
      cell.status = SeatStatus::Occupied;
    }
    m_students[s.id] = s;
    if (s.id >= nextId)
      nextId = s.id + 1;
  }
  emit dataChanged();
}

void HallModel::saveToCSV(const QString &path) const {
  QFile file(path);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    return;
  QTextStream ts(&file);

  // Header
  ts << "name,registrationNo,rollNo,program,section,department,subject,"
        "semester,isPhysicallyImpaired\n";

  // Data rows
  for (const auto &s : m_students) {
    ts << s.name << "," << s.registrationNo << "," << s.rollNumber << ","
       << s.program << "," << s.section << "," << s.department << ","
       << s.subject << "," << s.semester << ","
       << (s.isPhysicallyImpaired ? "true" : "false") << "\n";
  }
}

void HallModel::loadFromCSV(const QString &path) {
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    return;
  QTextStream ts(&file);

  // Clear existing students
  m_students.clear();
  nextId = 1;

  // Skip header line
  if (!ts.atEnd())
    ts.readLine();

  while (!ts.atEnd()) {
    QString line = ts.readLine().trimmed();
    if (line.isEmpty())
      continue;

    QStringList fields;
    QString current;
    bool inQuotes = false;
    for (int i = 0; i < line.length(); ++i) {
      QChar c = line[i];
      if (c == '"') {
        inQuotes = !inQuotes;
      } else if (c == ',' && !inQuotes) {
        fields.append(current.trimmed());
        current.clear();
      } else {
        current.append(c);
      }
    }
    fields.append(current.trimmed());

    if (fields.size() < 8)
      continue;

    UIStudent s;
    s.id = nextId++;
    s.name = fields[0];
    s.registrationNo = fields[1];
    s.rollNumber = fields[2];
    s.program = fields[3];

    if (fields.size() == 8) {
        s.section = "";
        s.department = fields[4];
        s.subject = fields[5];
        s.semester = fields[6].toInt();
        s.isPhysicallyImpaired = (fields[7].toLower() == "true" || fields[7] == "1");
    } else {
        s.section = fields[4];
        s.department = fields[5];
        s.subject = fields[6];
        s.semester = fields[7].toInt();
        s.isPhysicallyImpaired = (fields[8].toLower() == "true" || fields[8] == "1");
    }
    s.status = SeatStatus::Empty;
    m_students[s.id] = s;
  }
  emit dataChanged();
}
