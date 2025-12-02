package com.riptidez.notebackend.mapper.log;

import com.riptidez.notebackend.entity.log.Log;
import org.apache.ibatis.annotations.Mapper;

@Mapper
public interface LogMapper {
    int insert(Log log);
}
