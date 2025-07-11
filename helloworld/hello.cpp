#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <ctime>
#include <map>
#include <set>
#include <numeric>

// 基础人员类（抽象基类）
class Person {
protected:
    std::string name;
    int age;
    std::string gender;
    std::string id;

public:
    Person(const std::string& name, int age, const std::string& gender, const std::string& id)
        : name(name), age(age), gender(gender), id(id) {}

    virtual ~Person() = default;

    // 纯虚函数，子类必须实现
    virtual void introduce() const = 0;
    virtual std::string getRole() const = 0;

    // 普通成员函数
    std::string getName() const { return name; }
    int getAge() const { return age; }
    std::string getGender() const { return gender; }
    std::string getId() const { return id; }

    void setAge(int newAge) { age = newAge; }

    // 静态成员函数
    static std::string getSpecies() { return "Homo sapiens"; }
};

// 学生类
class Student : public Person {
private:
    std::string studentNumber;
    std::string major;
    double gpa;
    std::vector<std::string> courses;
    std::map<std::string, int> grades;
    bool isActive;
    int graduationYear;

public:
    Student(const std::string& name, int age, const std::string& gender, const std::string& id,
            const std::string& studentNumber, const std::string& major, int graduationYear)
        : Person(name, age, gender, id), studentNumber(studentNumber), major(major),
          gpa(0.0), isActive(true), graduationYear(graduationYear) {}

    // 重写虚函数
    void introduce() const override {
        std::cout << "大家好，我是学生 " << name << "，学号：" << studentNumber
                  << "，专业：" << major << "，GPA：" << gpa << std::endl;
    }

    std::string getRole() const override {
        return "学生";
    }

    // 添加课程
    void addCourse(const std::string& course) {
        courses.push_back(course);
    }

    // 添加成绩
    void addGrade(const std::string& course, int grade) {
        grades[course] = grade;
        calculateGPA();
    }

    // 计算GPA
    void calculateGPA() {
        if (grades.empty()) {
            gpa = 0.0;
            return;
        }

        int total = std::accumulate(grades.begin(), grades.end(), 0,
                                   [](int sum, const std::pair<std::string, int>& p) {
                                       return sum + p.second;
                                   });
        gpa = static_cast<double>(total) / grades.size();
    }

    // 获取所有课程
    const std::vector<std::string>& getCourses() const { return courses; }

    // 获取成绩
    int getGrade(const std::string& course) const {
        auto it = grades.find(course);
        return it != grades.end() ? it->second : 0;
    }

    // 获取学生信息
    std::string getStudentNumber() const { return studentNumber; }
    std::string getMajor() const { return major; }
    double getGPA() const { return gpa; }
    bool getActiveStatus() const { return isActive; }
    int getGraduationYear() const { return graduationYear; }

    // 设置状态
    void setActiveStatus(bool status) { isActive = status; }

    // 显示所有成绩
    void showAllGrades() const {
        std::cout << name << " 的成绩单：" << std::endl;
        for (const auto& grade : grades) {
            std::cout << "  " << grade.first << ": " << grade.second << "分" << std::endl;
        }
        std::cout << "  平均GPA: " << gpa << std::endl;
    }
};

// 教师类
class Teacher : public Person {
private:
    std::string employeeId;
    std::string department;
    std::string title;
    double salary;
    std::vector<std::string> courses;
    std::vector<std::shared_ptr<Student>> students;
    int yearsOfExperience;

public:
    Teacher(const std::string& name, int age, const std::string& gender, const std::string& id,
            const std::string& employeeId, const std::string& department, const std::string& title,
            double salary, int yearsOfExperience)
        : Person(name, age, gender, id), employeeId(employeeId), department(department),
          title(title), salary(salary), yearsOfExperience(yearsOfExperience) {}

    // 重写虚函数
    void introduce() const override {
        std::cout << "大家好，我是 " << title << " " << name << "，来自 " << department
                  << " 系，工作经验：" << yearsOfExperience << " 年" << std::endl;
    }

    std::string getRole() const override {
        return "教师";
    }

    // 添加课程
    void addCourse(const std::string& course) {
        courses.push_back(course);
    }

    // 添加学生
    void addStudent(std::shared_ptr<Student> student) {
        students.push_back(student);
    }

    // 给学生打分
    void gradeStudent(const std::string& studentNumber, const std::string& course, int grade) {
        for (auto& student : students) {
            if (student->getStudentNumber() == studentNumber) {
                student->addGrade(course, grade);
                std::cout << "已为学生 " << student->getName() << " 在课程 "
                          << course << " 中打分：" << grade << " 分" << std::endl;
                return;
            }
        }
        std::cout << "未找到学号为 " << studentNumber << " 的学生" << std::endl;
    }

    // 获取信息
    std::string getEmployeeId() const { return employeeId; }
    std::string getDepartment() const { return department; }
    std::string getTitle() const { return title; }
    double getSalary() const { return salary; }
    int getYearsOfExperience() const { return yearsOfExperience; }

    // 设置信息
    void setSalary(double newSalary) { salary = newSalary; }
    void setTitle(const std::string& newTitle) { title = newTitle; }

    // 显示所有学生
    void showAllStudents() const {
        std::cout << name << " 老师的学生列表：" << std::endl;
        for (const auto& student : students) {
            std::cout << "  " << student->getName() << " (学号："
                      << student->getStudentNumber() << ")" << std::endl;
        }
    }

    // 显示所有课程
    void showAllCourses() const {
        std::cout << name << " 老师教授的课程：" << std::endl;
        for (const auto& course : courses) {
            std::cout << "  " << course << std::endl;
        }
    }
};

// 课程类
class Course {
private:
    std::string courseCode;
    std::string courseName;
    int credits;
    std::string description;
    std::shared_ptr<Teacher> teacher;
    std::vector<std::shared_ptr<Student>> enrolledStudents;
    std::map<std::string, int> schedule; // 星期 -> 课时
    int maxCapacity;

public:
    Course(const std::string& code, const std::string& name, int credits,
           const std::string& description, int maxCapacity)
        : courseCode(code), courseName(name), credits(credits),
          description(description), maxCapacity(maxCapacity) {}

    // 设置授课教师
    void setTeacher(std::shared_ptr<Teacher> t) {
        teacher = t;
        if (teacher) {
            teacher->addCourse(courseName);
        }
    }

    // 添加学生
    bool enrollStudent(std::shared_ptr<Student> student) {
        if (enrolledStudents.size() >= maxCapacity) {
            std::cout << "课程 " << courseName << " 已满，无法添加学生" << std::endl;
            return false;
        }

        enrolledStudents.push_back(student);
        student->addCourse(courseName);
        std::cout << "学生 " << student->getName() << " 已成功选课 " << courseName << std::endl;
        return true;
    }

    // 设置课程时间表
    void setSchedule(const std::string& day, int hours) {
        schedule[day] = hours;
    }

    // 获取信息
    std::string getCourseCode() const { return courseCode; }
    std::string getCourseName() const { return courseName; }
    int getCredits() const { return credits; }
    std::string getDescription() const { return description; }
    int getEnrolledCount() const { return enrolledStudents.size(); }
    int getMaxCapacity() const { return maxCapacity; }

    // 显示课程信息
    void showCourseInfo() const {
        std::cout << "=== 课程信息 ===" << std::endl;
        std::cout << "课程代码：" << courseCode << std::endl;
        std::cout << "课程名称：" << courseName << std::endl;
        std::cout << "学分：" << credits << std::endl;
        std::cout << "描述：" << description << std::endl;
        std::cout << "授课教师：" << (teacher ? teacher->getName() : "待定") << std::endl;
        std::cout << "选课人数：" << enrolledStudents.size() << "/" << maxCapacity << std::endl;

        if (!schedule.empty()) {
            std::cout << "上课时间：" << std::endl;
            for (const auto& s : schedule) {
                std::cout << "  " << s.first << ": " << s.second << " 课时" << std::endl;
            }
        }
    }

    // 显示选课学生
    void showEnrolledStudents() const {
        std::cout << courseName << " 选课学生名单：" << std::endl;
        for (const auto& student : enrolledStudents) {
            std::cout << "  " << student->getName() << " (学号："
                      << student->getStudentNumber() << ")" << std::endl;
        }
    }
};

// 学校类
class School {
private:
    std::string schoolName;
    std::string address;
    std::vector<std::shared_ptr<Student>> students;
    std::vector<std::shared_ptr<Teacher>> teachers;
    std::vector<std::shared_ptr<Course>> courses;
    std::map<std::string, std::string> departments;
    int establishedYear;

public:
    School(const std::string& name, const std::string& address, int establishedYear)
        : schoolName(name), address(address), establishedYear(establishedYear) {}

    // 添加学生
    void addStudent(std::shared_ptr<Student> student) {
        students.push_back(student);
        std::cout << "学生 " << student->getName() << " 已加入 " << schoolName << std::endl;
    }

    // 添加教师
    void addTeacher(std::shared_ptr<Teacher> teacher) {
        teachers.push_back(teacher);
        std::cout << "教师 " << teacher->getName() << " 已加入 " << schoolName << std::endl;
    }

    // 添加课程
    void addCourse(std::shared_ptr<Course> course) {
        courses.push_back(course);
        std::cout << "课程 " << course->getCourseName() << " 已添加到 " << schoolName << std::endl;
    }

    // 添加院系
    void addDepartment(const std::string& deptCode, const std::string& deptName) {
        departments[deptCode] = deptName;
    }

    // 查找学生
    std::shared_ptr<Student> findStudent(const std::string& studentNumber) {
        for (auto& student : students) {
            if (student->getStudentNumber() == studentNumber) {
                return student;
            }
        }
        return nullptr;
    }

    // 查找教师
    std::shared_ptr<Teacher> findTeacher(const std::string& employeeId) {
        for (auto& teacher : teachers) {
            if (teacher->getEmployeeId() == employeeId) {
                return teacher;
            }
        }
        return nullptr;
    }

    // 查找课程
    std::shared_ptr<Course> findCourse(const std::string& courseCode) {
        for (auto& course : courses) {
            if (course->getCourseCode() == courseCode) {
                return course;
            }
        }
        return nullptr;
    }

    // 显示学校信息
    void showSchoolInfo() const {
        std::cout << "=== 学校信息 ===" << std::endl;
        std::cout << "学校名称：" << schoolName << std::endl;
        std::cout << "学校地址：" << address << std::endl;
        std::cout << "建校年份：" << establishedYear << std::endl;
        std::cout << "学生人数：" << students.size() << std::endl;
        std::cout << "教师人数：" << teachers.size() << std::endl;
        std::cout << "课程数量：" << courses.size() << std::endl;
        std::cout << "院系数量：" << departments.size() << std::endl;
    }

    // 显示所有院系
    void showAllDepartments() const {
        std::cout << schoolName << " 院系列表：" << std::endl;
        for (const auto& dept : departments) {
            std::cout << "  " << dept.first << ": " << dept.second << std::endl;
        }
    }

    // 显示统计信息
    void showStatistics() const {
        std::cout << "=== 学校统计信息 ===" << std::endl;

        // 学生平均GPA
        double totalGPA = 0.0;
        int activeStudents = 0;
        for (const auto& student : students) {
            if (student->getActiveStatus()) {
                totalGPA += student->getGPA();
                activeStudents++;
            }
        }

        if (activeStudents > 0) {
            std::cout << "在校学生平均GPA：" << totalGPA / activeStudents << std::endl;
        }

        // 教师平均工作经验
        int totalExperience = 0;
        for (const auto& teacher : teachers) {
            totalExperience += teacher->getYearsOfExperience();
        }

        if (!teachers.empty()) {
            std::cout << "教师平均工作经验：" << static_cast<double>(totalExperience) / teachers.size() << " 年" << std::endl;
        }

        // 课程平均选课人数
        int totalEnrolled = 0;
        for (const auto& course : courses) {
            totalEnrolled += course->getEnrolledCount();
        }

        if (!courses.empty()) {
            std::cout << "课程平均选课人数：" << static_cast<double>(totalEnrolled) / courses.size() << " 人" << std::endl;
        }
    }

    // 获取信息
    std::string getSchoolName() const { return schoolName; }
    std::string getAddress() const { return address; }
    int getEstablishedYear() const { return establishedYear; }

    // 获取历史（计算学校存在年数）
    int getSchoolAge() const {
        std::time_t now = std::time(nullptr);
        std::tm* now_tm = std::localtime(&now);
        return now_tm->tm_year + 1900 - establishedYear;
    }
};

// 实用工具类
class Utility {
public:
    // 生成随机学号
    static std::string generateStudentNumber() {
        std::string number = "2024";
        for (int i = 0; i < 6; i++) {
            number += std::to_string(rand() % 10);
        }
        return number;
    }

    // 生成随机教工号
    static std::string generateEmployeeId() {
        std::string id = "T";
        for (int i = 0; i < 5; i++) {
            id += std::to_string(rand() % 10);
        }
        return id;
    }

    // 生成随机课程代码
    static std::string generateCourseCode() {
        std::string code = "CS";
        for (int i = 0; i < 3; i++) {
            code += std::to_string(rand() % 10);
        }
        return code;
    }

    // 格式化输出分割线
    static void printSeparator(const std::string& title = "") {
        std::cout << "\n" << std::string(50, '=') << std::endl;
        if (!title.empty()) {
            std::cout << title << std::endl;
            std::cout << std::string(50, '=') << std::endl;
        }
    }
};

// 主函数演示
int main() {
    std::cout << "=== 面向对象学校管理系统演示 ===" << std::endl;

    // 创建学校
    auto school = std::make_shared<School>("清华大学", "北京市海淀区清华园1号", 1911);

    // 添加院系
    school->addDepartment("CS", "计算机科学与技术系");
    school->addDepartment("EE", "电子工程系");
    school->addDepartment("MATH", "数学科学系");

    Utility::printSeparator("创建学生");

    // 创建学生
    auto student1 = std::make_shared<Student>("张三", 20, "男", "110101200301011234",
                                             Utility::generateStudentNumber(), "计算机科学与技术", 2024);
    auto student2 = std::make_shared<Student>("李四", 19, "女", "110101200401011234",
                                             Utility::generateStudentNumber(), "电子工程", 2025);
    auto student3 = std::make_shared<Student>("王五", 21, "男", "110101200201011234",
                                             Utility::generateStudentNumber(), "数学", 2023);

    // 添加学生到学校
    school->addStudent(student1);
    school->addStudent(student2);
    school->addStudent(student3);

    Utility::printSeparator("创建教师");

    // 创建教师
    auto teacher1 = std::make_shared<Teacher>("陈教授", 45, "男", "110101197801011234",
                                             Utility::generateEmployeeId(), "计算机科学与技术系",
                                             "教授", 15000.0, 20);
    auto teacher2 = std::make_shared<Teacher>("刘副教授", 38, "女", "110101198501011234",
                                             Utility::generateEmployeeId(), "电子工程系",
                                             "副教授", 12000.0, 12);

    // 添加教师到学校
    school->addTeacher(teacher1);
    school->addTeacher(teacher2);

    Utility::printSeparator("创建课程");

    // 创建课程
    auto course1 = std::make_shared<Course>(Utility::generateCourseCode(), "数据结构与算法",
                                           3, "计算机科学基础课程", 50);
    auto course2 = std::make_shared<Course>(Utility::generateCourseCode(), "数字电路",
                                           4, "电子工程基础课程", 40);
    auto course3 = std::make_shared<Course>(Utility::generateCourseCode(), "高等数学",
                                           5, "数学基础课程", 60);

    // 设置课程教师
    course1->setTeacher(teacher1);
    course2->setTeacher(teacher2);
    course3->setTeacher(teacher1);

    // 设置课程时间表
    course1->setSchedule("周一", 2);
    course1->setSchedule("周三", 2);
    course2->setSchedule("周二", 3);
    course2->setSchedule("周四", 1);
    course3->setSchedule("周五", 4);

    // 添加课程到学校
    school->addCourse(course1);
    school->addCourse(course2);
    school->addCourse(course3);

    Utility::printSeparator("学生选课");

    // 学生选课
    course1->enrollStudent(student1);
    course1->enrollStudent(student2);
    course2->enrollStudent(student2);
    course3->enrollStudent(student1);
    course3->enrollStudent(student3);

    Utility::printSeparator("教师给学生打分");

    // 教师给学生打分
    teacher1->addStudent(student1);
    teacher1->addStudent(student2);
    teacher1->addStudent(student3);
    teacher2->addStudent(student2);

    teacher1->gradeStudent(student1->getStudentNumber(), "数据结构与算法", 92);
    teacher1->gradeStudent(student1->getStudentNumber(), "高等数学", 88);
    teacher1->gradeStudent(student2->getStudentNumber(), "数据结构与算法", 95);
    teacher2->gradeStudent(student2->getStudentNumber(), "数字电路", 90);
    teacher1->gradeStudent(student3->getStudentNumber(), "高等数学", 85);

    Utility::printSeparator("显示各种信息");

    // 显示学校信息
    school->showSchoolInfo();
    std::cout << std::endl;

    // 显示院系信息
    school->showAllDepartments();
    std::cout << std::endl;

    // 显示学生信息
    std::cout << "=== 学生信息 ===" << std::endl;
    student1->introduce();
    student1->showAllGrades();
    std::cout << std::endl;

    student2->introduce();
    student2->showAllGrades();
    std::cout << std::endl;

    student3->introduce();
    student3->showAllGrades();
    std::cout << std::endl;

    // 显示教师信息
    std::cout << "=== 教师信息 ===" << std::endl;
    teacher1->introduce();
    teacher1->showAllStudents();
    teacher1->showAllCourses();
    std::cout << std::endl;

    teacher2->introduce();
    teacher2->showAllStudents();
    teacher2->showAllCourses();
    std::cout << std::endl;

    // 显示课程信息
    std::cout << "=== 课程信息 ===" << std::endl;
    course1->showCourseInfo();
    course1->showEnrolledStudents();
    std::cout << std::endl;

    course2->showCourseInfo();
    course2->showEnrolledStudents();
    std::cout << std::endl;

    course3->showCourseInfo();
    course3->showEnrolledStudents();
    std::cout << std::endl;

    // 显示统计信息
    school->showStatistics();

    Utility::printSeparator("演示多态性");

    // 演示多态性
    std::vector<std::shared_ptr<Person>> people;
    people.push_back(student1);
    people.push_back(student2);
    people.push_back(teacher1);
    people.push_back(teacher2);

    std::cout << "所有人员介绍：" << std::endl;
    for (const auto& person : people) {
        std::cout << "角色：" << person->getRole() << std::endl;
        person->introduce();
        std::cout << std::endl;
    }

    std::cout << "程序演示完成！" << std::endl;

    return 0;
}