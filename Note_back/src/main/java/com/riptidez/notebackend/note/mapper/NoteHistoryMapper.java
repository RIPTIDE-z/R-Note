package com.riptidez.notebackend.note.mapper;

import com.riptidez.notebackend.note.entity.NoteHistory;
import org.apache.ibatis.annotations.Mapper;
import org.apache.ibatis.annotations.Param;

import java.util.List;

@Mapper
public interface NoteHistoryMapper {
    int insert(NoteHistory history);

    List<NoteHistory> getNoteHistoryWithoutContentListByNoteId(@Param("noteId") Long noteId);

    NoteHistory getNoteHistoryByNoteIdAndVersion(@Param("noteId") Long noteId,
                                                 @Param("version") Integer version);

    NoteHistory getLatestNoteHistoryByNoteId(@Param("noteId") Long noteId);
}
