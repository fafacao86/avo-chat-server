package nwpu.se.avoserver.exception;

import nwpu.se.avoserver.constant.ResultCodeEnum;

/**
 * 业务异常
 * 构造函数参数是ResultCodeEnum，说明业务异常有多种类型
 * 会返回HTTP 200,但是会在response的body部分标注请求失败及失败原因
 */
public class BusinessException extends RuntimeException {

    private final int code;

    public BusinessException(ResultCodeEnum resultCodeEnum) {
        super(resultCodeEnum.getMessage());
        this.code = resultCodeEnum.getCode();
    }


    public BusinessException(ResultCodeEnum resultCodeEnum, String message) {
        super(message);
        this.code = resultCodeEnum.getCode();
    }

    public int getCode() {
        return code;
    }


}
