package nwpu.se.avoserver.advice;

import nwpu.se.avoserver.vo.CommonResult;
import com.fasterxml.jackson.databind.ObjectMapper;
import lombok.SneakyThrows;
import lombok.extern.slf4j.Slf4j;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.core.MethodParameter;
import org.springframework.http.MediaType;
import org.springframework.http.server.ServerHttpRequest;
import org.springframework.http.server.ServerHttpResponse;
import org.springframework.web.bind.annotation.RestControllerAdvice;
import org.springframework.web.servlet.mvc.method.annotation.ResponseBodyAdvice;

/**
 * 这个类是用来包装Controller方法的返回结果的，不管Controller返回什么内容，这个类都能按照标准返回模板进行包装
 * 有个坑要注意：
 * 如果在Controller的方法参数列表里有HttpServletResponse，且方法返回为void或null，会导致这个包装器失效。（这真是一个奇怪的问题）
 * 所以一旦在Controller的方法里用到HttpServletResponse，而且又没东西想返回的适合，建议返回一个new Object();总之别返回void或者null
 */
@RestControllerAdvice(basePackages = "nwpu.se.avoserver.controller")
@Slf4j
public class CommonResultWrapper implements ResponseBodyAdvice<Object> {

    @Autowired
    private ObjectMapper objectMapper;

    /**
     * 设置这个“包装器”是否启用。return true表示启用。
     */
    @Override
    public boolean supports(MethodParameter returnType, Class converterType) {
        return true;
    }

    /**
     * 自定义包装方法，根据Controller层返回的内容不同，有不同的包装方法。
     * 不过最终要给前端返回的是标准返回模板————CommonResult
     */
    @Override
    @SneakyThrows
    public Object beforeBodyWrite(Object body, MethodParameter returnType, MediaType selectedContentType, Class selectedConverterType, ServerHttpRequest request, ServerHttpResponse response) {
        if (body == null) {
            return CommonResult.success();
        }
        if (body instanceof String) {
            return objectMapper.writeValueAsString(CommonResult.success(body));
        }
        if (body instanceof CommonResult) {
            return body;
        }
        return CommonResult.success(body);
    }
}

