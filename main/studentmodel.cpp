#include "../include/studentmodel.h"
#include <QJsonDocument>
#include <algorithm>
#include <random>

HallModel::HallModel(QObject *parent) : QObject(parent) {
    // zero-init all seats
    for (int b=0;b<9;b++)
        for (int s=0;s<3;s++)
            for (int n=0;n<6;n++)
                for (int t=0;t<2;t++)
                    m_seats[b][s][n][t] = SeatCell{};
}

int HallModel::blockIndex(const QString &block) const {
    return HallConst::BLOCK_NAMES.indexOf(block.toUpper());
}

// ── CRUD ────────────────────────────────────────────────────────────────────
void HallModel::addStudent(const Student &s) {
    Student ns = s;
    if (ns.id <= 0) ns.id = nextId++;
    else if (ns.id >= nextId) nextId = ns.id + 1;
    m_students[ns.id] = ns;
    emit dataChanged();
}

void HallModel::updateStudent(const Student &s) {
    if (m_students.contains(s.id)) {
        m_students[s.id] = s;
        emit dataChanged();
    }
}

void HallModel::removeStudent(int id) {
    if (!m_students.contains(id)) return;
    Student &st = m_students[id];
    if (st.isAssigned()) {
        unassignSeat(st.block, st.subBlock, st.bench, st.seat);
    }
    m_students.remove(id);
    emit dataChanged();
}

Student* HallModel::findById(int id) {
    auto it = m_students.find(id);
    return (it != m_students.end()) ? &it.value() : nullptr;
}

Student* HallModel::findByRoll(const QString &roll) {
    for (auto &s : m_students)
        if (s.rollNumber.compare(roll, Qt::CaseInsensitive)==0)
            return &s;
    return nullptr;
}

QList<Student*> HallModel::searchStudents(const QString &q) const {
    QList<Student*> results;
    QString ql = q.toLower();
    for (auto &s : m_students) {
        if (s.name.toLower().contains(ql) || s.rollNumber.toLower().contains(ql) ||
            s.department.toLower().contains(ql))
            results.append(const_cast<Student*>(&s));
    }
    return results;
}

QList<Student*> HallModel::allStudents() {
    QList<Student*> list;
    for (auto &s : m_students) list.append(&s);
    return list;
}

QList<Student*> HallModel::studentsByBlock(const QString &block) {
    QList<Student*> list;
    for (auto &s : m_students)
        if (s.block == block) list.append(&s);
    return list;
}

QList<Student*> HallModel::studentsByDept(const QString &dept) {
    QList<Student*> list;
    for (auto &s : m_students)
        if (s.department == dept) list.append(&s);
    return list;
}

QList<Student*> HallModel::unassignedStudents() {
    QList<Student*> list;
    for (auto &s : m_students)
        if (!s.isAssigned()) list.append(&s);
    return list;
}

// ── Seat Access ─────────────────────────────────────────────────────────────
SeatCell& HallModel::seatAt(const QString &block, int sub, int bench, int seat) {
    int bi = blockIndex(block);
    return m_seats[bi][sub-1][bench-1][seat-1];
}
const SeatCell& HallModel::seatAt(const QString &block, int sub, int bench, int seat) const {
    int bi = blockIndex(block);
    return m_seats[bi][sub-1][bench-1][seat-1];
}

// ── Assignment ───────────────────────────────────────────────────────────────
bool HallModel::assignSeat(int studentId, const QString &block, int sub, int bench, int seat) {
    Student *st = findById(studentId);
    if (!st) return false;
    SeatCell &cell = seatAt(block, sub, bench, seat);
    if (cell.locked) return false;
    if (cell.studentId != -1 && cell.studentId != studentId) return false;

    // unassign previous seat if any
    if (st->isAssigned())
        unassignSeat(st->block, st->subBlock, st->bench, st->seat);

    cell.studentId = studentId;
    cell.status = SeatStatus::Occupied;
    st->block    = block;
    st->subBlock = sub;
    st->bench    = bench;
    st->seat     = seat;
    st->status   = SeatStatus::Occupied;

    emit seatAssigned(studentId, st->seatCode());
    emit dataChanged();
    return true;
}

bool HallModel::unassignSeat(const QString &block, int sub, int bench, int seat) {
    SeatCell &cell = seatAt(block, sub, bench, seat);
    if (cell.locked) return false;
    if (cell.studentId != -1) {
        Student *st = findById(cell.studentId);
        if (st) {
            st->block.clear(); st->subBlock=0; st->bench=0; st->seat=0;
            st->status = SeatStatus::Empty;
        }
    }
    cell = SeatCell{};
    emit dataChanged();
    return true;
}

void HallModel::clearAllAssignments() {
    for (int b=0;b<9;b++)
        for (int s=0;s<3;s++)
            for (int n=0;n<6;n++)
                for (int t=0;t<2;t++)
                    if (!m_seats[b][s][n][t].locked)
                        m_seats[b][s][n][t] = SeatCell{};
    for (auto &s : m_students) {
        s.block.clear(); s.subBlock=0; s.bench=0; s.seat=0;
        s.status = SeatStatus::Empty;
    }
    emit dataChanged();
}

bool HallModel::autoAssign() {
    QList<Student*> unassigned = unassignedStudents();
    if (unassigned.isEmpty()) return true;
    int ui = 0;
    for (int b=0; b<9 && ui<unassigned.size(); b++) {
        for (int s=0; s<3 && ui<unassigned.size(); s++) {
            for (int n=0; n<6 && ui<unassigned.size(); n++) {
                for (int t=0; t<2 && ui<unassigned.size(); t++) {
                    SeatCell &cell = m_seats[b][s][n][t];
                    if (cell.studentId == -1 && !cell.locked) {
                        assignSeat(unassigned[ui]->id,
                                   HallConst::BLOCK_NAMES[b], s+1, n+1, t+1);
                        ui++;
                    }
                }
            }
        }
    }
    return (ui == unassigned.size());
}

bool HallModel::lockSeat(const QString &block, int sub, int bench, int seat, bool lock) {
    SeatCell &cell = seatAt(block, sub, bench, seat);
    if (lock && cell.studentId != -1) return false; // can't lock occupied
    cell.locked = lock;
    emit dataChanged();
    return true;
}

// ── Capacity ─────────────────────────────────────────────────────────────────
int HallModel::occupiedCount() const {
    int c = 0;
    for (int b=0;b<9;b++)
        for (int s=0;s<3;s++)
            for (int n=0;n<6;n++)
                for (int t=0;t<2;t++)
                    if (m_seats[b][s][n][t].studentId != -1) c++;
    return c;
}

int HallModel::occupiedInBlock(const QString &block) const {
    int bi = blockIndex(block);
    if (bi<0) return 0;
    int c=0;
    for (int s=0;s<3;s++)
        for (int n=0;n<6;n++)
            for (int t=0;t<2;t++)
                if (m_seats[bi][s][n][t].studentId != -1) c++;
    return c;
}

int HallModel::occupiedInSubBlock(const QString &block, int sub) const {
    int bi = blockIndex(block);
    if (bi<0) return 0;
    int c=0;
    for (int n=0;n<6;n++)
        for (int t=0;t<2;t++)
            if (m_seats[bi][sub-1][n][t].studentId != -1) c++;
    return c;
}

double HallModel::utilizationPercent() const {
    return (double)occupiedCount() / HallConst::CAPACITY * 100.0;
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

QList<QPair<int,int>> HallModel::detectConflicts() {
    QList<QPair<int,int>> conflicts;
    // Simple: same dept same semester adjacent seats
    // For now just mark duplicates
    QMap<QString,int> seatMap;
    for (auto &s : m_students) {
        if (!s.isAssigned()) continue;
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
    QStringList depts = {"Computer Science","Civil Engineering","Electrical Engineering",
                         "Mechanical Engineering","Architecture","Electronics","Chemical Engineering"};
    QStringList names = {
        "Aarav Sharma","Bikash Thapa","Chandra Poudel","Dipika Rai","Elina Karki",
        "Firoj Magar","Gita Tamang","Hari Basnet","Isha Shrestha","Jeevan Adhikari",
        "Kabita Gurung","Laxman Bhatt","Maya Neupane","Nabin Dahal","Ojasvi Joshi",
        "Prakash Khadka","Quresh Ali","Rajan Bhandari","Sunita Chand","Tulsi Bhusal",
        "Uma Koirala","Vivek Pandey","Wangchen Lama","Xena Sthapit","Yashoda Dhakal",
        "Bijaya Kumar","Anita Sapkota","Roshan Ghimire","Priya Acharya","Sunil Thakur",
        "Deepak Kafle","Nisha Rijal","Kiran Subedi","Puja Regmi","Arjun Mishra",
        "Sita Pariyar","Mohan Bista","Rekha Chaudhary","Anil Mainali","Binod Humagain",
        "Champa Limbu","Dev Shahi","Eva Maharjan","Faisal Ansari","Gaurav Bhattarai",
        "Hema Pandey","Ishan Kharel","Jaya Giri","Krishna Chhetri","Lalita Raut"
    };

    int rollBase = 20001;
    for (int i=0; i<names.size(); i++) {
        Student s;
        s.id         = nextId++;
        s.name       = names[i];
        s.rollNumber = QString("KU%1").arg(rollBase + i);
        s.department = depts[i % depts.size()];
        s.semester   = (i % 8) + 1;
        s.program    = (i % 3 == 0) ? "BE" : (i % 3 == 1) ? "BArch" : "BSc";
        s.subject    = QString("Subject %1").arg((i % 5) + 1);
        s.status     = SeatStatus::Empty;
        m_students[s.id] = s;
    }
    emit dataChanged();
}

QJsonObject HallModel::toJson() const {
    QJsonObject root;
    QJsonArray students;
    for (auto &s : m_students) {
        QJsonObject obj;
        obj["id"]         = s.id;
        obj["name"]       = s.name;
        obj["roll"]       = s.rollNumber;
        obj["dept"]       = s.department;
        obj["semester"]   = s.semester;
        obj["program"]    = s.program;
        obj["subject"]    = s.subject;
        obj["block"]      = s.block;
        obj["subBlock"]   = s.subBlock;
        obj["bench"]      = s.bench;
        obj["seat"]       = s.seat;
        students.append(obj);
    }
    root["students"] = students;
    return root;
}

void HallModel::fromJson(const QJsonObject &obj) {
    m_students.clear();
    for (int b=0;b<9;b++)
        for (int s=0;s<3;s++)
            for (int n=0;n<6;n++)
                for (int t=0;t<2;t++)
                    m_seats[b][s][n][t] = SeatCell{};
    nextId = 1;
    QJsonArray students = obj["students"].toArray();
    for (auto v : students) {
        QJsonObject o = v.toObject();
        Student s;
        s.id         = o["id"].toInt();
        s.name       = o["name"].toString();
        s.rollNumber = o["roll"].toString();
        s.department = o["dept"].toString();
        s.semester   = o["semester"].toInt();
        s.program    = o["program"].toString();
        s.subject    = o["subject"].toString();
        s.block      = o["block"].toString();
        s.subBlock   = o["subBlock"].toInt();
        s.bench      = o["bench"].toInt();
        s.seat       = o["seat"].toInt();
        if (s.isAssigned()) {
            s.status = SeatStatus::Occupied;
            SeatCell &cell = seatAt(s.block, s.subBlock, s.bench, s.seat);
            cell.studentId = s.id;
            cell.status = SeatStatus::Occupied;
        }
        m_students[s.id] = s;
        if (s.id >= nextId) nextId = s.id+1;
    }
    emit dataChanged();
}
