package com.riptidez.notebackend.service.studentbiz;

import com.riptidez.notebackend.entity.log.Log;
import com.riptidez.notebackend.entity.student.Student;

public interface StudentBizService {
    void addStudentWithLog(Student student, Log log);
}
