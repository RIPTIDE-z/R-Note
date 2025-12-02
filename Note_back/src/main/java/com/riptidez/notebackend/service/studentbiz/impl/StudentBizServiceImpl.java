package com.riptidez.notebackend.service.studentbiz.impl;

import com.riptidez.notebackend.entity.log.Log;
import com.riptidez.notebackend.entity.student.Student;
import com.riptidez.notebackend.service.log.LogService;
import com.riptidez.notebackend.service.student.StudentService;
import com.riptidez.notebackend.service.studentbiz.StudentBizService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

@Service
public class StudentBizServiceImpl implements StudentBizService {
    @Autowired
    private StudentService studentService;

    @Autowired
    private LogService logService;

    @Transactional(rollbackFor = Exception.class)
    public void addStudentWithLog(Student student, Log log){
        // 1. 调用 StudentService，插入 student_db
        boolean ok1 = studentService.addStudent(student);
        if (!ok1) {
            // 必须抛异常才能触发回滚
            throw new RuntimeException("插入 student_db 失败");
        }

        // 2. 调用 LogService，写 log_db
        boolean ok2 = logService.addLog(student.getId());
        if (!ok2) {
            throw new RuntimeException("写 log_db 日志失败");
        }

        // 3. 测试分布式回滚：打开下面这一行
        throw new RuntimeException("手动抛异常，验证 Atomikos 分布式回滚");
    }

}
