package com.riptidez.notebackend.note.controller;

import com.riptidez.notebackend.auth.helper.AuthTokenHelper;
import com.riptidez.notebackend.auth.service.SessionService;
import com.riptidez.notebackend.exception.ExceptionWithMessage;
import com.riptidez.notebackend.note.dto.UpdateNoteRequestDto;
import com.riptidez.notebackend.note.dto.UpdateNoteResponseDto;
import com.riptidez.notebackend.note.entity.Note;
import com.riptidez.notebackend.note.entity.NoteHistory;
import com.riptidez.notebackend.note.service.NoteHistoryService;
import com.riptidez.notebackend.note.service.NoteService;
import com.riptidez.notebackend.note.service.impl.NoteHistoryServiceImpl;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.*;

import java.util.List;

/**
 * 笔记及历史相关接口。
 * <p>
 * Base URL: /notes
 * <p>
 * 接口列表：
 * <pre>
 * POST /notes
 *   - 功能：为当前登录用户创建一个空白笔记
 *   - 认证：请求头 Auth-Token: string
 *   - 请求体：{}
 *   - 响应：Note {
 *       id: long,
 *       userId: long,
 *       currentHistoryId: long
 *     }
 *
 * GET /notes/{noteId}
 *   - 功能：获取指定笔记的当前内容
 *   - 认证：请求头 Auth-Token: string
 *   - 路径参数：
 *       noteId: long
 *   - 响应：NoteHistory {
 *       id: long,
 *       noteId: long,
 *       version: int,
 *       createdTime: string,
 *       content: string,
 *       changeSummary: string
 *     }
 *
 * GET /notes/{noteId}/histories
 *   - 功能：获取指定笔记的历史版本列表（一般不返回 content）
 *   - 认证：请求头 Auth-Token: string
 *   - 路径参数：
 *       noteId: long
 *   - 响应：List<NoteHistory>，元素形如：
 *     {
 *       id: long,
 *       noteId: long,
 *       version: int,
 *       createdTime: string,
 *       changeSummary: string
 *     }
 *
 * GET /notes/{noteId}/histories/{version}
 *   - 功能：查看指定版本的完整内容
 *   - 认证：请求头 Auth-Token: string
 *   - 路径参数：
 *       noteId: long
 *       version: int
 *   - 响应：NoteHistory {
 *       id: long,
 *       noteId: long,
 *       version: int,
 *       createdTime: string,
 *       content: string,
 *       changeSummary: string
 *     }
 *
 * POST /notes/{noteId}/histories
 *   - 功能：基于指定版本创建一个新的当前版本（更新/回滚/新建）
 *   - 认证：请求头 Auth-Token: string
 *   - 路径参数：
 *       noteId: long
 *       version: int
 *   - 请求体：UpdateNoteRequestDto {
 *       changeSummary: string
 *     }
 *   - 响应：NoteHistory {
 *       id: long,
 *       noteId: long,
 *       version: int,
 *       createdTime: string,
 *       content: string,
 *       changeSummary: string
 *     }
 * </pre>
 */

@RestController
@RequestMapping("/notes")
public class NoteController {
    private final static Logger log = LoggerFactory.getLogger(NoteController.class);
    @Autowired
    private NoteService noteService;

    @Autowired
    private NoteHistoryService noteHistoryService;

    @Autowired
    private AuthTokenHelper authTokenHelper;


    @PostMapping
    public Note createNote(@RequestHeader("Auth-Token") String token) {
        Long userId = authTokenHelper.requireUserId(token);
        return noteService.createEmptyNote(userId);
    }

    @GetMapping("/{noteId}")
    public NoteHistory getNote(@RequestHeader("Auth-Token") String token,
                               @PathVariable Long noteId) {
        Long userId = authTokenHelper.requireUserId(token);
        return noteService.getNote(noteId, userId);
    }

    @GetMapping("/{noteId}/histories")
    public List<NoteHistory> listHistories(@RequestHeader("Auth-Token") String token,
                                           @PathVariable Long noteId) {
        Long userId = authTokenHelper.requireUserId(token);
        return noteHistoryService.listHistories(noteId, userId);
    }

    @GetMapping("/{noteId}/histories/{version}")
    public NoteHistory getHistory(@RequestHeader("Auth-Token") String token,
                                  @PathVariable Long noteId,
                                  @PathVariable Long version) {
        Long userId = authTokenHelper.requireUserId(token);
        return noteHistoryService.getHistory(noteId, version, userId);
    }

    @PostMapping("/{noteId}/histories")
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
            // 业务异常（你在 service 里自己 throw 的）
            return UpdateNoteResponseDto.fail(e.getMessage(), null);
        }
    }
}
