package com.riptidez.notebackend.controller;

import com.riptidez.notebackend.entity.student.Student;
import com.riptidez.notebackend.entity.log.Log;
import com.riptidez.notebackend.service.student.StudentService;
import com.riptidez.notebackend.service.studentbiz.StudentBizService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * 学生管理控制器
 */
@RestController
@RequestMapping("/students")
@CrossOrigin(origins = "*")
public class StudentController {

    private static final Logger log = LoggerFactory.getLogger(StudentController.class);

    @Autowired
    private StudentService studentService;

    @Autowired
    private StudentBizService StudentBizService;

    /**
     * 查询所有学生
     */
    @GetMapping
    public ResponseEntity<Map<String, Object>> getAllStudents() {
        log.info("正在查询学生...");
        List<Student> students = studentService.getAllStudents();
        Map<String, Object> response = new HashMap<>();
        response.put("code", 200);
        response.put("message", "查询成功");
        response.put("data", students);
        log.info("查询成功！");
        return ResponseEntity.ok(response);
    }

    /**
     * 根据ID查询学生
     */
    @GetMapping("/{id}")
    public ResponseEntity<Map<String, Object>> getStudentById(@PathVariable Integer id) {
        log.info("正在查询学生{}", id);
        Student student = studentService.getStudentById(id);
        Map<String, Object> response = new HashMap<>();
        if (student != null) {
            response.put("code", 200);
            response.put("message", "查询成功");
            response.put("data", student);
            log.info("查询成功");
        } else {
            response.put("code", 404);
            response.put("message", "学生不存在");
            log.info("查询失败，学生{}不存在", id);
        }
        return ResponseEntity.ok(response);
    }

    /**
     * 添加学生
     */
    @PostMapping
    public ResponseEntity<Map<String, Object>> addStudent(@RequestBody Student student) {
        boolean success = studentService.addStudent(student);
        Map<String, Object> response = new HashMap<>();
        response.put("code", success ? 200 : 500);
        response.put("message", success ? "添加成功" : "添加失败");
        response.put("data", student);
        return ResponseEntity.ok(response);
    }

    /**
     * 添加学生的同时增加一条日志，用于测试Atomikos
     */
    @PostMapping("/with-log")
    public ResponseEntity<Map<String, Object>> addStudentWithLog(@RequestBody Student student) {
        Map<String, Object> response = new HashMap<>();
        try {
            // 构造一条日志（只需要前端发 Student，也可以后端自己决定写什么日志）
            Log log = new Log();

            // 调用带分布式事务的业务方法：
            // 内部：student_db 插入 + log_db 插入日志
            StudentBizService.addStudentWithLog(student, log);

            response.put("code", 200);
            response.put("message", "添加成功（已记录日志）");
            response.put("data", student);
        } catch (Exception e) {
            // 事务已经在 Service 层回滚，这里只负责返回错误信息
            response.put("code", 500);
            response.put("message", "添加失败（已回滚）: " + e.getMessage());
            response.put("data", null);
        }
        return ResponseEntity.ok(response);
    }

    /**
     * 更新学生信息
     */
    @PutMapping("/{id}")
    public ResponseEntity<Map<String, Object>> updateStudent(
            @PathVariable Integer id, @RequestBody Student student) {
        student.setId(id);
        boolean success = studentService.updateStudent(student);
        Map<String, Object> response = new HashMap<>();
        response.put("code", success ? 200 : 500);
        response.put("message", success ? "更新成功" : "更新失败");
        return ResponseEntity.ok(response);
    }

    /**
     * 删除学生
     */
    @DeleteMapping("/{id}")
    public ResponseEntity<Map<String, Object>> deleteStudent(@PathVariable Integer id) {
        boolean success = studentService.deleteStudent(id);
        Map<String, Object> response = new HashMap<>();
        response.put("code", success ? 200 : 500);
        response.put("message", success ? "删除成功" : "删除失败");
        return ResponseEntity.ok(response);
    }
}