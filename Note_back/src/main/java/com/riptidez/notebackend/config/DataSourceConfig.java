package com.riptidez.notebackend.config;

import com.alibaba.druid.pool.xa.DruidXADataSource;
import com.atomikos.jdbc.AtomikosDataSourceBean;
import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.boot.context.properties.ConfigurationProperties;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.context.annotation.Primary;


/**
 * 两个 XA DataSource的配置（DruidXA + Atomikos）
 */
@Configuration
public class DataSourceConfig {

    // ===================== auth_db =====================

    @Bean(name = "authDruidXADataSource")
    @ConfigurationProperties(prefix = "spring.datasource.druid.auth-db")
    public DruidXADataSource authDruidXADataSource() {
        return new DruidXADataSource();
    }

    @Primary
    @Bean(name = "authDataSource", initMethod = "init", destroyMethod = "close")
    @ConfigurationProperties(prefix = "spring.jta.atomikos.datasource.auth")
    public AtomikosDataSourceBean authDataSource(
            @Qualifier("authDruidXADataSource") DruidXADataSource druidXADataSource) {

        AtomikosDataSourceBean xaDataSource = new AtomikosDataSourceBean();
        xaDataSource.setXaDataSource(druidXADataSource);
        xaDataSource.setUniqueResourceName("authDbXA");

        return xaDataSource;
    }

    // ===================== note_db =====================

    @Bean(name = "noteDruidXADataSource")
    @ConfigurationProperties(prefix = "spring.datasource.druid.note-db")
    public DruidXADataSource noteDruidXADataSource() {
        return new DruidXADataSource();
    }

    @Bean(name = "noteDataSource", initMethod = "init", destroyMethod = "close")
    @ConfigurationProperties(prefix = "spring.jta.atomikos.datasource.note")
    public AtomikosDataSourceBean noteDataSource(
            @Qualifier("noteDruidXADataSource") DruidXADataSource druidXADataSource) {

        AtomikosDataSourceBean xaDataSource = new AtomikosDataSourceBean();
        xaDataSource.setXaDataSource(druidXADataSource);
        xaDataSource.setUniqueResourceName("noteDbXA");

        return xaDataSource;
    }
}
