package com.riptidez.notebackend.auth.mapper;

import com.riptidez.notebackend.auth.entity.User;
import org.apache.ibatis.annotations.Mapper;
import org.apache.ibatis.annotations.Param;

@Mapper
public interface UserMapper {
    User findHashByUsername(@Param("username") String username);

    User findById(@Param("id") Long id);

    int insert(User user);

    int updateNoteStructure(@Param("id") Long id,
                            @Param("noteStructure") String noteStructure);
}
