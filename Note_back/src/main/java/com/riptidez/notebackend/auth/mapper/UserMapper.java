package com.riptidez.notebackend.auth.mapper;

import com.riptidez.notebackend.auth.entity.User;
import org.apache.ibatis.annotations.Mapper;
import org.apache.ibatis.annotations.Param;

@Mapper
public interface UserMapper {
    User getHashByUsername(@Param("username") String username);
    User getNoteStructureByUserId(@Param("id") Long id);

    User findUserById(@Param("id") Long id);

    int insert(User user);

    int updateNoteStructure(@Param("id") Long id,
                            @Param("noteStructure") String noteStructure);
}
