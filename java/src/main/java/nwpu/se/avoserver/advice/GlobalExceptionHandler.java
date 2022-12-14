package nwpu.se.avoserver.advice;



import com.fasterxml.jackson.databind.ObjectMapper;
import lombok.SneakyThrows;
import lombok.extern.slf4j.Slf4j;
import nwpu.se.avoserver.constant.ResultCodeEnum;
import nwpu.se.avoserver.exception.BusinessException;
import nwpu.se.avoserver.exception.IllegalOperationException;
import nwpu.se.avoserver.exception.NotFoundException;
import nwpu.se.avoserver.vo.CommonResult;
import org.hibernate.validator.internal.engine.path.PathImpl;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.HttpStatus;
import org.springframework.http.converter.HttpMessageNotReadableException;
import org.springframework.security.access.AccessDeniedException;
import org.springframework.validation.BindException;
import org.springframework.web.HttpRequestMethodNotSupportedException;
import org.springframework.web.bind.MissingRequestHeaderException;
import org.springframework.web.bind.ServletRequestBindingException;
import org.springframework.web.bind.annotation.ExceptionHandler;
import org.springframework.web.bind.annotation.ResponseStatus;
import org.springframework.web.bind.annotation.RestControllerAdvice;
import org.springframework.web.method.annotation.MethodArgumentTypeMismatchException;
import org.springframework.web.servlet.NoHandlerFoundException;

import javax.validation.ConstraintViolationException;

/**
 * 全局异常处理器
 * 需要注意的是，这个处理器[只能处理经过Filter之后才发生的异常]，它可以处理Interceptor抛出的异常、Controller抛出的异常、Service抛出的异常、Mapper抛出的异常。
 * 对于在Filter中抛出的异常，例如认证失败导致抛出异常，不在这里处理，而是有专门的处理器（详情请见SecurityConfig类）
 * 请求的顺序：Filter(过滤器)->Interceptor(拦截器)->Controller->Service->Mapper
 * 有人也管Mapper叫做DAO层
 */
@RestControllerAdvice
@Slf4j
public class GlobalExceptionHandler {

    @Autowired
    private ObjectMapper objectMapper;


    /**
     * 参数校验失败【针对于请求中Body部分的校验失败，方法形参要加@RequestBody来指明处理的Body部分，
     * 别和下面Path部分和Query部分搞混，这俩抛出的异常是不一样的】
     * 校验Body的Controller方法样例：search(@RequestBody @Valid UserParam userParam)
     * 提示：想校验Body参数，就算不在Controller类上加@Validated注解也能生效（当然加了也不碍事，为了省事，干脆把所有Controller都加上@Validated）
     */
    @ExceptionHandler(BindException.class)
    public CommonResult handleBindException(BindException e) {
        String message = formatBindException(e);
        log.warn(formatException(e, message, false));
        return CommonResult.failure(ResultCodeEnum.PARAM_VALIDATE_FAILED.getCode(), message);
    }


    /**
     * 参数校验失败【针对于请求中Path部分和Query部分的校验失败，方法形参不加@RequestBody
     * （如果是Path参数则要加@PathVariable，Query参数则不用额外加注解），别和上面Body部分搞混，这俩抛出的异常是不一样的】
     * 校验Query的Controller方法样例：search(@Valid @NotBlank(message= "姓名不能为空") String name)，并且务必在Controller类上加@Validated
     * 校验Path的Controller方法样例：search(@PathVariable @Valid @Min(value = 1,message = "用户ID最小是1")  Long userId)，并且务必在Controller类上加@Validated
     * 特别注意：如果你想校验Query参数或者Path参数，必须要在Controller类上加@Validated注解，否则校验不生效
     */
    @ExceptionHandler(ConstraintViolationException.class)
    public CommonResult handleBindException(ConstraintViolationException e) {
        String message = formatConstraintViolationException(e);
        log.warn(formatException(e, message, false));
        return CommonResult.failure(ResultCodeEnum.PARAM_VALIDATE_FAILED.getCode(), message);
    }


    /**
     * 请求方式不支持
     */
    @ResponseStatus(HttpStatus.METHOD_NOT_ALLOWED)
    @ExceptionHandler(HttpRequestMethodNotSupportedException.class)
    public CommonResult handleMethodNotAllowed(Exception e) {
        log.warn(formatException(e, null, false));
        return CommonResult.failure("请求方式不支持");
    }


    /**
     * 请求格式不对
     */
    @ResponseStatus(HttpStatus.BAD_REQUEST)
    @ExceptionHandler({ServletRequestBindingException.class, MissingRequestHeaderException.class, MethodArgumentTypeMismatchException.class, HttpMessageNotReadableException.class})
    public CommonResult handleBadRequest(Exception e) {
        log.warn(formatException(e, null, false));
        return CommonResult.failure("请求格式不对");
    }


    /**
     * 请求的接口权限不足，例如一个普通用户去请求管理员才能用的接口（这是授权的时候抛出的异常，在进入Controller之前就被拦住，更不会进入Service层）
     */
    @ResponseStatus(HttpStatus.FORBIDDEN)
    @ExceptionHandler(AccessDeniedException.class)
    public CommonResult handleAccessDeniedException(Exception e) {
        log.warn(formatException(e, null, false));
        return CommonResult.failure("权限不足");
    }

    /**
     * 请求的操作非法，例如一个普通用户要删除其他普通用户发布的内容（已通过授权并进入了Controller层，但是在Service层发现操作非法而抛出的异常）
     */
    @ResponseStatus(HttpStatus.FORBIDDEN)
    @ExceptionHandler(IllegalOperationException.class)
    public CommonResult handleIllegalOperationException(Exception e) {
        log.warn(formatException(e, null, false));
        return CommonResult.failure("操作非法");
    }

    /**
     * 请求URL有误，无法解析这个URL该对应Controller中哪个方法
     */
    @ResponseStatus(HttpStatus.NOT_FOUND)
    @ExceptionHandler(NoHandlerFoundException.class)
    public CommonResult handleNotFoundException(NoHandlerFoundException e) {
        log.warn(formatException(e, null, false));
        return CommonResult.failure("请求URL不存在");
    }

    /**
     * 请求资源不存在
     */
    @ResponseStatus(HttpStatus.NOT_FOUND)
    @ExceptionHandler(NotFoundException.class)
    public CommonResult handleNotFoundException(Exception e) {
        log.warn(formatException(e, null, true));
        return CommonResult.failure("请求资源不存在");
    }

    /**
     * 业务异常，可细分为多种情况，可见ResultCodeEnum
     */
    @ExceptionHandler(BusinessException.class)
    public CommonResult handleBusinessException(BusinessException e) {
        log.warn(formatException(e, null, true));
        return CommonResult.failure(e.getCode(), e.getMessage());
    }


    /**
     * 如果前面的处理器都没拦截住，最后兜底
     */
    @ResponseStatus(HttpStatus.INTERNAL_SERVER_ERROR)
    @ExceptionHandler(Exception.class)
    public CommonResult handleException(Exception e) {
        log.warn(formatException(e, null, true));
        return CommonResult.failure("服务器内部错误");
    }


    /**
     * 把异常信息格式化成自己喜欢的格式，这个方法用于格式化Exception
     */
    @SneakyThrows
    public String formatException(Exception e, String message, boolean stackRequired) {
        StringBuilder sb = new StringBuilder();
        sb.append("(异常)")
                .append("<类型>").append(e.getClass())
                .append("<信息>").append(message != null ? message : e.getMessage());
        if (stackRequired) {
            sb.append("<堆栈>").append(objectMapper.writeValueAsString(e));
        }
        return sb.toString();
    }


    /**
     * 把异常信息格式化成自己喜欢的格式，这个方法用于格式化BindException
     */
    public static String formatBindException(BindException e) {
        StringBuilder sb = new StringBuilder();
        e.getBindingResult().getFieldErrors().forEach(
                error -> {
                    //提示：error.getField()得到的是校验失败的字段名字，error.getDefaultMessage()得到的是校验失败的原因
                    sb.append(error.getField()).append("=[").append(error.getDefaultMessage()).append("]  ");
                }
        );
        return sb.toString();
    }

    /**
     * 把异常信息格式化成自己喜欢的格式，这个方法用于格式化ConstraintViolationException
     */
    public static String formatConstraintViolationException(ConstraintViolationException e) {
        StringBuilder sb = new StringBuilder();
        e.getConstraintViolations().forEach(
                violation -> {
                    //提示：((PathImpl) violation.getPropertyPath()).getLeafNode().getName()得到的是校验失败的字段名字，violation.getMessage()得到的是校验失败的原因
                    sb.append(((PathImpl) violation.getPropertyPath()).getLeafNode().getName()).append("=[").append(violation.getMessage()).append("]  ");
                });
        return sb.toString();
    }
}
