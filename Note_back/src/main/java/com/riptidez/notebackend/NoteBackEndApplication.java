package com.riptidez.notebackend;

import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;

@SpringBootApplication
public class NoteBackEndApplication {
    public static void main(String[] args) {
        SpringApplication.run(NoteBackEndApplication.class, args);
        System.out.println("========================================");
        System.out.println("✅ 笔记后端系统启动成功！");
        System.out.println("========================================");
    }
}