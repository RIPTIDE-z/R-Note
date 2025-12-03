package com.riptidez.notebackend.exception;

public class ExceptionWithMessage extends RuntimeException {
    public ExceptionWithMessage(String message) {
        super(message);
    }
}
