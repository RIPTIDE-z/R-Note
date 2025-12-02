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
 * 主数据源：auth_db
 * 配置 MyBatis（SqlSessionFactory / SqlSessionTemplate / MapperScan）
 */
@Configuration
@MapperScan(
        basePackages = "com.riptidez.notebackend.auth.mapper",
        sqlSessionTemplateRef = "authSqlSessionTemplate"
)
public class PrimaryDataSourceConfig {

    @Primary
    @Bean(name = "authSqlSessionFactory")
    public SqlSessionFactory authSqlSessionFactory(
            @Qualifier("authDataSource") DataSource dataSource) throws Exception {

        SqlSessionFactoryBean bean = new SqlSessionFactoryBean();
        bean.setDataSource(dataSource);
        bean.setMapperLocations(
                new PathMatchingResourcePatternResolver()
                        .getResources("classpath*:mapper/auth/*.xml"));
        bean.setTypeAliasesPackage("com.riptidez.notebackend.auth.entity");
        return bean.getObject();
    }

    @Primary
    @Bean(name = "authSqlSessionTemplate")
    public SqlSessionTemplate authSqlSessionTemplate(
            @Qualifier("authSqlSessionFactory") SqlSessionFactory sqlSessionFactory) {
        return new SqlSessionTemplate(sqlSessionFactory);
    }
}
