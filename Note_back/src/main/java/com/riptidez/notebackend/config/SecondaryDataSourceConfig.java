package com.riptidez.notebackend.config;

import org.apache.ibatis.session.SqlSessionFactory;
import org.mybatis.spring.SqlSessionFactoryBean;
import org.mybatis.spring.SqlSessionTemplate;
import org.mybatis.spring.annotation.MapperScan;
import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.core.io.support.PathMatchingResourcePatternResolver;

import javax.sql.DataSource;

/**
 * 次数据源：log_db
 * 配置 MyBatis（SqlSessionFactory / SqlSessionTemplate / MapperScan）
 */
@Configuration
@MapperScan(
        basePackages = "com.riptidez.notebackend.mapper.log",
        sqlSessionTemplateRef = "logSqlSessionTemplate"
)
public class SecondaryDataSourceConfig {

    @Bean(name = "logSqlSessionFactory")
    public SqlSessionFactory logSqlSessionFactory(
            @Qualifier("logDataSource") DataSource dataSource) throws Exception {

        SqlSessionFactoryBean bean = new SqlSessionFactoryBean();
        bean.setDataSource(dataSource);
        bean.setMapperLocations(
                new PathMatchingResourcePatternResolver()
                        .getResources("classpath*:mapper/log/*.xml"));
        bean.setTypeAliasesPackage("com.riptidez.notebackend.entity.log");
        return bean.getObject();
    }

    @Bean(name = "logSqlSessionTemplate")
    public SqlSessionTemplate logSqlSessionTemplate(
            @Qualifier("logSqlSessionFactory") SqlSessionFactory sqlSessionFactory) {

        return new SqlSessionTemplate(sqlSessionFactory);
    }
}
