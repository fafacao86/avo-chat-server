package nwpu.se.avoserver.security;


import nwpu.se.avoserver.security.TokenFilter;
import nwpu.se.avoserver.security.AuthenticationFailedHandler;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.security.authentication.AuthenticationManager;
import org.springframework.security.config.annotation.authentication.configuration.AuthenticationConfiguration;
import org.springframework.security.config.annotation.method.configuration.EnableGlobalMethodSecurity;
import org.springframework.security.config.annotation.web.builders.HttpSecurity;
import org.springframework.security.config.http.SessionCreationPolicy;
import org.springframework.security.crypto.bcrypt.BCryptPasswordEncoder;
import org.springframework.security.web.SecurityFilterChain;
import org.springframework.security.web.authentication.UsernamePasswordAuthenticationFilter;

/**
 * 这是SpringSecurity的配置类，因为WebSecurityConfigurerAdapter在SpringSecurity 2.7 及以上版本以上被标注为弃用，所以这里不再使用Adapter
 * 以下是文档：
 * <a href="https://spring.io/blog/2022/02/21/spring-security-without-the-websecurityconfigureradapter">官方更新说明</a>
 * <a href="https://github.com/spring-projects/spring-security/issues/10822">github上相关issues</a>
 */
@Configuration
@EnableGlobalMethodSecurity(prePostEnabled = true)
public class SecurityConfig {

    /**
     * 下面这个方法用于配置"保护策略"
     * 在不同的项目中，不拦截URL列表改通常是不同的
     */
    @Bean
    public SecurityFilterChain filterChain(HttpSecurity http, AuthenticationFailedHandler authenticationFailedHandler, TokenFilter tokenFilter) throws Exception {
        //.csrf().disable()以及.sessionManagement().sessionCreationPolicy(SessionCreationPolicy.STATELESS);是在配置两个不一样的东西
        //csrf是默认开启的，我们需要关闭csrf才行。
        // csrf要求请求携带“_csrf”字段，否则会被拦截，我们前端发的请求都没有“_csrf”，不应该启动这个功能
        //不要用session获取context，因为JWT的优点就是不需要session
        //SessionCreationPolicy.STATELESS就是指禁用session
        http.csrf().disable().sessionManagement().sessionCreationPolicy(SessionCreationPolicy.STATELESS);

        //配置哪些路径需要认证，哪些路径不需要认证
        //配置的顺序很重要，越靠前的规则优先级越高
        //在这个项目中，我们设置了三个URL无需认证，剩下的所有URL都要认证
        //通俗地讲：访问下面这三个URL，无需token
        //但是，你访问地路径如果不是这三个之一，那么就必须带上合法的token，以便我能知道“你是谁”
        //不要搞混认证和授权，先认证再授权，认证在前，授权在后。
        //认证是指要知道“你是谁，你是不是我们系统的用户”，不论你是普通用户还是管理员，都算认证成功，
        //授权是指要知道“你有哪些权限，你是否拥有要访问的接口要求的权限“，必须有要求的权限，才算授权成功
        //以/actuator开头的URL是为了暴露运行信息，要直接放行。可以通过/actuator/health查看是否健康，通过/actuator/prometheus查看指标
        http.authorizeRequests().antMatchers(
                        "/user/login",
                        "/user/register",
                        "/actuator/**"
                ).permitAll()
                .anyRequest().authenticated();

        //TokenFilter是我们自定义的一个Filter，我想把它加入过滤器链里面，所以调用addFilterBefore
        //如果你不调用addFilterBefore方法，那么我们写的Filter是不会生效的
        //TokenFilter的详细功能请见那个类里面注释
        http.addFilterBefore(tokenFilter, UsernamePasswordAuthenticationFilter.class);

        //这里是配置认证失败处理器
        //认证失败而抛出的异常是在Filter中抛出的，全局异常处理器是[捕捉不到在Filter中抛出的异常的]，因为全局异常处理器只能捕捉到经过Filter之后才出现的异常
        //我们用authenticationEntryPoint方法指定在Filter中抛出的异常由谁去处理，可以认为在这里指定“认证失败”抛出的异常由谁去处理
        http.exceptionHandling().authenticationEntryPoint(authenticationFailedHandler);
        //允许跨域
        http.cors();

        return http.build();
    }

    /**
     * 往项目中注入BCryptPasswordEncoder，这是SpringSecurity写好的工具类，
     * 它实现了PasswordEncoder接口，可以用它来把密码加密，这样就不用我们自己写加密的逻辑了
     */
    @Bean
    public BCryptPasswordEncoder bCryptPasswordEncoder() {
        return new BCryptPasswordEncoder();
    }

    /**
     * 为IOC容器中注入AuthenticationManager，因为UserServiceImpl要用
     */
    @Bean
    public AuthenticationManager authenticationManager(AuthenticationConfiguration authenticationConfiguration) throws Exception {
        return authenticationConfiguration.getAuthenticationManager();
    }


}
