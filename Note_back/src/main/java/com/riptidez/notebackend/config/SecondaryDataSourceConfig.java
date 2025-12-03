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
 * 次数据源：note_db 的 MyBatis 配置
 */
@Configuration
@MapperScan(
        basePackages = "com.riptidez.notebackend.note.mapper",
        sqlSessionTemplateRef = "noteSqlSessionTemplate"
)
public class SecondaryDataSourceConfig {

    @Bean(name = "noteSqlSessionFactory")
    public SqlSessionFactory noteSqlSessionFactory(
            @Qualifier("noteDataSource") DataSource dataSource) throws Exception {

        SqlSessionFactoryBean bean = new SqlSessionFactoryBean();
        bean.setDataSource(dataSource);
        bean.setMapperLocations(
                new PathMatchingResourcePatternResolver()
                        .getResources("classpath*:mapper/note/*.xml"));
        bean.setTypeAliasesPackage("com.riptidez.notebackend.note.entity");
        return bean.getObject();
    }

    @Bean(name = "noteSqlSessionTemplate")
    public SqlSessionTemplate noteSqlSessionTemplate(
            @Qualifier("noteSqlSessionFactory") SqlSessionFactory sqlSessionFactory) {

        return new SqlSessionTemplate(sqlSessionFactory);
    }
}
