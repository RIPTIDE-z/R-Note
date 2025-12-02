package com.riptidez.notebackend.service.log.impl;

import com.riptidez.notebackend.entity.log.Log;
import com.riptidez.notebackend.mapper.log.LogMapper;
import com.riptidez.notebackend.service.log.LogService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.time.LocalDateTime;

@Service
public class LogServiceImpl implements LogService {
    @Autowired
    private LogMapper logMapper;

    @Override
    public boolean addLog(Integer studentId){
        Log log = new Log();
        log.setStudentId(studentId);
        log.setOperationTime(LocalDateTime.now());

        return logMapper.insert(log) > 0;
    }
}
