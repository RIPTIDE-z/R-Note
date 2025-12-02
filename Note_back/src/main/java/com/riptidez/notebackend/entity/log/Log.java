package com.riptidez.notebackend.entity.log;
import java.time.LocalDateTime;

/**
 * 日志实体类
 */
public class Log {
    private Long id;
    private Integer studentId;
    private LocalDateTime operationTime;

    public Log() {}
    public Log(Integer studentId, LocalDateTime operationTime) {
        this.studentId = studentId;
        this.operationTime = operationTime;
    }

    public Long getId() {
        return id;
    }

    public void setId(Long id) {
        this.id = id;
    }

    public Integer getStudentId() {
        return studentId;
    }

    public void setStudentId(Integer studentId) {
        this.studentId = studentId;
    }

    public LocalDateTime getOperationTime() {
        return operationTime;
    }

    public void setOperationTime(LocalDateTime operationTime) {
        this.operationTime = operationTime;
    }

    @Override
    public String toString() {
        return "Log{" +
                "id=" + id +
                ", studentId=" + studentId +
                ", operationTime=" + operationTime +
                '}';
    }
}
