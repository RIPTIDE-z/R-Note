package com.riptidez.notebackend.note.controller;

import com.riptidez.notebackend.auth.helper.AuthTokenHelper;
import com.riptidez.notebackend.exception.ExceptionWithMessage;
import com.riptidez.notebackend.note.dto.UpdateNoteRequestDto;
import com.riptidez.notebackend.note.dto.UpdateNoteResponseDto;
import com.riptidez.notebackend.note.entity.NoteHistory;
import com.riptidez.notebackend.note.service.NoteService;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.*;

import java.util.List;

@RestController
@RequestMapping("/notes")
public class NoteController {
    @Autowired
    private NoteService noteHistoryService;

    @Autowired
    private AuthTokenHelper authTokenHelper;

    // 完全删除一个笔记，包括历史版本
    // 注意Note和NoteHistory表创建时自带的删除约束
    @DeleteMapping("/{noteId}")
    public UpdateNoteResponseDto deleteNote(@RequestHeader("Auth-Token") String token,
                                            @PathVariable Long noteId) {
        Long userId = authTokenHelper.requireUserId(token);
        try {
            noteHistoryService.deleteNote(noteId, userId);
            return UpdateNoteResponseDto.success("笔记删除成功", null);
        } catch (ExceptionWithMessage e) {
            return UpdateNoteResponseDto.fail(e.getMessage(), null);
        }
    }

    // 获取不带实际内容的笔记历史列表，用于创建历史版本大纲
    @GetMapping("/{noteId}/histories")
    public List<NoteHistory> listHistories(@RequestHeader("Auth-Token") String token,
                                           @PathVariable Long noteId) {
        Long userId = authTokenHelper.requireUserId(token);
        return noteHistoryService.listHistories(noteId, userId);
    }

    // 获取指定笔记的指定版本的内容
    @GetMapping("/{noteId}/{version}")
    public NoteHistory getHistory(@RequestHeader("Auth-Token") String token,
                                  @PathVariable Long noteId,
                                  @PathVariable Integer version) {
        Long userId = authTokenHelper.requireUserId(token);
        return noteHistoryService.getHistory(noteId, version, userId);
    }

    // 对未同步笔记进行新建，对已同步的笔记进行更新和回滚
    @PostMapping("/{noteId}")
    public UpdateNoteResponseDto updateNote(@RequestHeader("Auth-Token") String token,
                                            @PathVariable(required = false) Long noteId,
                                            @RequestBody UpdateNoteRequestDto dto) {
        Long userId = authTokenHelper.requireUserId(token);

        try {
            Integer Code = dto.getCode();
            if (Code == null) {
                return UpdateNoteResponseDto.fail("code 不能为空", null);
            }

            switch (Code) {
                case 0: // 更新
                    if (noteId == null) {
                        return UpdateNoteResponseDto.fail("更新失败 noteId 不能为空",  null);
                    }
                    if (dto.getContent() == null) {
                        return UpdateNoteResponseDto.fail("更新失败 笔记内容不能为空",  null);
                    }
                    noteHistoryService.updateNote(
                            noteId,
                            userId,
                            dto.getContent(),
                            dto.getChangeSummary()
                    );
                    return UpdateNoteResponseDto.success("笔记更新成功", null);

                case 1: // 回滚
                    if (noteId == null || dto.getTargetVersion() == null) {
                        return UpdateNoteResponseDto.fail("回滚操作 noteId / targetVersion 不能为空", null);
                    }
                    noteHistoryService.rollbackNote(
                            noteId,
                            dto.getTargetVersion(),
                            userId,
                            dto.getChangeSummary()
                    );
                    return UpdateNoteResponseDto.success("笔记回滚成功",  null);

                case 2: // 新建
                    if (dto.getContent() == null) {
                        return UpdateNoteResponseDto.fail("新建笔记失败 笔记内容不能为空",   null);
                    }
                    NoteHistory created = noteHistoryService.createNote(
                            userId,
                            dto.getContent(),
                            (dto.getChangeSummary() != null && !dto.getChangeSummary().isEmpty())
                                    ? dto.getChangeSummary()
                                    : "note init"
                    );
                    // 如果你想让前端拿到新 noteId，可以顺带拼在 message 里
                    return UpdateNoteResponseDto.success("新建笔记成功，noteId=" + created.getNoteId(), created.getNoteId());

                default:
                    return UpdateNoteResponseDto.fail("不支持的 code=" + Code,  null);
            }

        } catch (ExceptionWithMessage e) {
            return UpdateNoteResponseDto.fail(e.getMessage(), null);
        }
    }
}
