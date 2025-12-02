package com.riptidez.notebackend.mapper.student;

import com.riptidez.notebackend.entity.student.Student;
import org.apache.ibatis.annotations.Mapper;
import java.util.List;

@Mapper
public interface StudentMapper {
    List<Student> selectAll();
    Student selectById(Integer id);
    int insert(Student student);
    int update(Student student);
    int deleteById(Integer id);
}