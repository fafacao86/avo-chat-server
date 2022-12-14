package nwpu.se.avoserver.security;


import com.alibaba.fastjson.JSONObject;
import lombok.extern.slf4j.Slf4j;
import nwpu.se.avoserver.common.JwtUtil;
import nwpu.se.avoserver.entity.User;
import nwpu.se.avoserver.exception.IllegalOperationException;
import org.slf4j.MDC;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.security.authentication.BadCredentialsException;
import org.springframework.security.authentication.UsernamePasswordAuthenticationToken;
import org.springframework.security.core.context.SecurityContextHolder;
import org.springframework.stereotype.Component;
import org.springframework.util.StringUtils;
import org.springframework.web.filter.OncePerRequestFilter;

import javax.servlet.FilterChain;
import javax.servlet.ServletException;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.io.BufferedReader;
import java.io.IOException;
import java.util.Objects;

/**
 * 这是一个Filter（过滤器），其职能为：
 * （1）如果请求没带token，则[不设置]Authentication标识为true，并且放行
 * （2）如果请求带了token，但是token是无效的，抛出异常，不会进入下一个Filter，异常会被认证失败处理器捕获，返回HTTP401
 * （3）如果请求带了token，而且token是有效的，[设置]Authentication标识为true，并且放行
 * 仅仅用@Component注入IOC容器还不够，还需要在SpringConfig里面配置
 * SpringSecurity是由一串过滤器链实现的，而这里就是我们自定义的一个过滤器
 */
@Component
@Slf4j
public class TokenFilter extends OncePerRequestFilter {

    @Autowired
    private JwtUtil jwtUtil;

    @Override
    protected void doFilterInternal(HttpServletRequest request, HttpServletResponse response, FilterChain filterChain) throws ServletException, IOException {

        String traceId = Objects.requireNonNullElse(request.getHeader("X-B3-Traceid"), "unset");
        MDC.put("traceId", traceId);
        response.addHeader("traceId", traceId);

        MDC.put("userId", "unknown");
        log.info("(请求)<方法>{}<URI>{}<Query>{}", request.getMethod(), request.getRequestURI(), request.getQueryString());
        String token = request.getHeader("token");
        //如果有token，就执行token解析，若解析成功，就设置Authenticated为true，表示认证成功
        if (StringUtils.hasText(token)) {
            User user;
            try {
                user = jwtUtil.getUserFromToken(token);
                //覆盖之前放入MDC的userId
                MDC.put("userId", user.getUserId().toString());
            }
            catch (Exception e) {
                log.warn("在解析token时出错，错误原因:{}", e.getMessage());
                throw new BadCredentialsException(e.getMessage());
            }

            SecurityContextHolder.getContext().setAuthentication(
                    new UsernamePasswordAuthenticationToken(user, null, user.getAuthorities())
            );

        }
        //如果token是空的，也必须放行，因为那些注册和登录接口就无需带token，如果按照没带token一律抛异常的逻辑，那么注册和登录接口就没法用了
        //那不带token会不会也进入需要认证的接口呢？其实不用担心。不带token的请求在这个filter中，不会设置布尔值“Authenticated”为true，
        //所以这个请求只有在访问无需认证接口（注册/登录）时不会被拦，在访问需要认证接口的时候会被拦
        //至于哪些接口属于无需认证，哪些接口需要认证，请见SecurityConfig类
        filterChain.doFilter(request, response);
    }

}
