package com.riptidez.notebackend.service.student;

import com.riptidez.notebackend.entity.student.Student;
import java.util.List;

public interface StudentService {
    List<Student> getAllStudents();
    Student getStudentById(Integer id);
    boolean addStudent(Student student);
    boolean updateStudent(Student student);
    boolean deleteStudent(Integer id);
}