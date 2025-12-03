package com.riptidez.notebackend.note.dto;

/**
 * 用于更新/回滚请求的请求体
 */
public class UpdateNoteRequestDto {
    /** 0表示更新 1表示回滚 */
    private Integer mode;
    /** 笔记内容 */
    private String content;
    /** 要回滚到的版本 */
    private Integer targetVersion;
    /** 更改说明 */
    private String changeSummary;

    public Integer getMode() {
        return mode;
    }

    public void setMode(Integer mode) {
        this.mode = mode;
    }

    public String getContent() {
        return content;
    }

    public void setContent(String content) {
        this.content = content;
    }

    public Integer getTargetVersion() {
        return targetVersion;
    }

    public void setTargetVersion(Integer targetVersion) {
        this.targetVersion = targetVersion;
    }

    public String getChangeSummary() {
        return changeSummary;
    }

    public void setChangeSummary(String changeSummary) {
        this.changeSummary = changeSummary;
    }

    @Override
    public String toString() {
        return "UpdateNoteRequestDto{" +
                "mode=" + mode +
                ", content='" + content + '\'' +
                ", targetVersion=" + targetVersion +
                ", changeSummary='" + changeSummary + '\'' +
                '}';
    }
}
