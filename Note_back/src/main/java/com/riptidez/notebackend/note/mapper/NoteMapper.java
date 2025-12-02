package com.riptidez.notebackend.note.mapper;

import com.riptidez.notebackend.note.entity.Note;
import org.apache.ibatis.annotations.Mapper;
import org.apache.ibatis.annotations.Param;

import java.util.List;

@Mapper
public interface NoteMapper {
    int insert(Note note);

    Note findById(@Param("id") Long id);

    List<Note> findByUserId(@Param("userId") Long userId);

    int updateCurrentHistoryId(@Param("id") Long id,
                               @Param("currentHistoryId") Long currentHistoryId);
}
