package com.riptidez.notebackend.config;

import org.apache.ibatis.session.SqlSessionFactory;
import org.mybatis.spring.SqlSessionFactoryBean;
import org.mybatis.spring.SqlSessionTemplate;
import org.mybatis.spring.annotation.MapperScan;
import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.context.annotation.Primary;
import org.springframework.core.io.support.PathMatchingResourcePatternResolver;

import javax.sql.DataSource;

/**
 * 主数据源：student_db
 * 配置 MyBatis（SqlSessionFactory / SqlSessionTemplate / MapperScan）
 */
@Configuration
@MapperScan(
        basePackages = "com.riptidez.notebackend.mapper.student",
        sqlSessionTemplateRef = "studentSqlSessionTemplate"
)
public class PrimaryDataSourceConfig {

    @Primary
    @Bean(name = "studentSqlSessionFactory")
    public SqlSessionFactory studentSqlSessionFactory(
            @Qualifier("studentDataSource") DataSource dataSource) throws Exception {

        SqlSessionFactoryBean bean = new SqlSessionFactoryBean();
        bean.setDataSource(dataSource);
        bean.setMapperLocations(
                new PathMatchingResourcePatternResolver()
                        .getResources("classpath*:mapper/student/*.xml"));
        bean.setTypeAliasesPackage("com.riptidez.notebackend.entity.student");
        return bean.getObject();
    }

    @Primary
    @Bean(name = "studentSqlSessionTemplate")
    public SqlSessionTemplate studentSqlSessionTemplate(
            @Qualifier("studentSqlSessionFactory") SqlSessionFactory sqlSessionFactory) {
        return new SqlSessionTemplate(sqlSessionFactory);
    }
}
