package nwpu.se.avoserver.security;

import com.fasterxml.jackson.databind.ObjectMapper;
import lombok.extern.slf4j.Slf4j;
import nwpu.se.avoserver.vo.CommonResult;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.HttpStatus;
import org.springframework.security.core.AuthenticationException;
import org.springframework.security.web.AuthenticationEntryPoint;
import org.springframework.stereotype.Component;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.io.IOException;

/**
 * 认证失败处理器，专门负责处理在Filter中抛出的异常
 */
@Component
@Slf4j
public class AuthenticationFailedHandler implements AuthenticationEntryPoint {

    @Autowired
    private ObjectMapper objectMapper;

    @Override
    public void commence(HttpServletRequest request, HttpServletResponse response, AuthenticationException authException) throws IOException {
        //设置HTTP状态码为401，表示认证失败
        log.warn("认证失败");
        response.setStatus(HttpStatus.UNAUTHORIZED.value());
        response.setCharacterEncoding("UTF-8");
        response.setContentType("application/json");
        response.getWriter().print(objectMapper.writeValueAsString(CommonResult.failure("[SpringSecurity] 认证失败，因为访问的接口需要token，但是token缺失/token无效/token过期")));
    }
}
