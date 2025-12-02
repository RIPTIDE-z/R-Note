package com.riptidez.notebackend.service.student.impl;

import com.riptidez.notebackend.entity.student.Student;
import com.riptidez.notebackend.mapper.student.StudentMapper;
import com.riptidez.notebackend.service.student.StudentService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;
import java.util.List;

@Service
public class StudentServiceImpl implements StudentService {

    @Autowired
    private StudentMapper studentMapper;

    @Override
    public List<Student> getAllStudents() {
        return studentMapper.selectAll();
    }

    @Override
    public Student getStudentById(Integer id) {
        return studentMapper.selectById(id);
    }

    @Override
    public boolean addStudent(Student student) { return studentMapper.insert(student) > 0;}

    @Override
    public boolean updateStudent(Student student) {
        return studentMapper.update(student) > 0;
    }

    @Override
    public boolean deleteStudent(Integer id) {
        return studentMapper.deleteById(id) > 0;
    }
}