package nwpu.se.avoserver.constant;

/**
 * 不同业务异常及其对应的状态码
 */
public enum ResultCodeEnum {

    /**
     * 参数校验失败
     */
    PARAM_VALIDATE_FAILED(10001, "参数校验失败"),
    /**
     * 用户名或密码错误（登录接口用）
     */
    WRONG_ID_OR_PASSWORD(10002, "ID或密码错误"),

    /**
     * 重复操作
     */
    REPEAT_OPERATION(10003, "重复操作"),

    /**
     * 服务器内部错误
     * */
    INTERNAL_ERROR(10004,"服务器内部错误");


    private final int code;
    private final String message;

    ResultCodeEnum(int code, String message) {
        this.code = code;
        this.message = message;
    }


    public int getCode() {
        return code;
    }

    public String getMessage() {
        return message;
    }
}
