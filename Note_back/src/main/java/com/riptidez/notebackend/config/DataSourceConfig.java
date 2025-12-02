package com.riptidez.notebackend.config;

import com.alibaba.druid.pool.xa.DruidXADataSource;
import com.atomikos.jdbc.AtomikosDataSourceBean;
import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.boot.context.properties.ConfigurationProperties;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.context.annotation.Primary;


/**
 * 只负责创建两个 XA DataSource（DruidXA + Atomikos）
 */
@Configuration
public class DataSourceConfig {

    // ===================== student_db =====================

    @Bean(name = "studentDruidXADataSource")
    @ConfigurationProperties(prefix = "spring.datasource.druid.student-db")
    public DruidXADataSource studentDruidXADataSource() {
        return new DruidXADataSource();
    }

    @Primary
    @Bean(name = "studentDataSource", initMethod = "init", destroyMethod = "close")
    @ConfigurationProperties(prefix = "spring.jta.atomikos.datasource.student")
    public AtomikosDataSourceBean studentDataSource(
            @Qualifier("studentDruidXADataSource") DruidXADataSource druidXADataSource) {

        AtomikosDataSourceBean xaDataSource = new AtomikosDataSourceBean();
        xaDataSource.setXaDataSource(druidXADataSource);
        xaDataSource.setUniqueResourceName("studentDbXA"); // 必须唯一

        return xaDataSource;
    }

    // ===================== log_db =====================

    @Bean(name = "logDruidXADataSource")
    @ConfigurationProperties(prefix = "spring.datasource.druid.log-db")
    public DruidXADataSource logDruidXADataSource() {
        return new DruidXADataSource();
    }

    @Bean(name = "logDataSource", initMethod = "init", destroyMethod = "close")
    @ConfigurationProperties(prefix = "spring.jta.atomikos.datasource.log")
    public AtomikosDataSourceBean logDataSource(
            @Qualifier("logDruidXADataSource") DruidXADataSource druidXADataSource) {

        AtomikosDataSourceBean xaDataSource = new AtomikosDataSourceBean();
        xaDataSource.setXaDataSource(druidXADataSource);
        xaDataSource.setUniqueResourceName("logDbXA");

        return xaDataSource;
    }
}
