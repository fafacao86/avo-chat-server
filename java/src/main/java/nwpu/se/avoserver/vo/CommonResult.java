package nwpu.se.avoserver.vo;


import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

/**
 * 返回模板
 * (其中success、code、message为必填，data为选填)
 */
@Data
@NoArgsConstructor
@AllArgsConstructor
public class CommonResult {

    /**
     * 是否成功（必填）
     */
    private Boolean success;

    /**
     * 业务状态码（必填）
     */
    private Integer code;

    /**
     * 对业务状态码的解释（必填）
     */
    private String message;

    /**
     * 返回数据（选填）
     */
    private Object data;


    public static CommonResult success() {
        return new CommonResult(true, 200, "操作成功", null);
    }

    public static CommonResult success(Object data) {
        return new CommonResult(true, 200, "操作成功", data);
    }

    public static CommonResult failure(String message) {
        return new CommonResult(false, -1, message, null);
    }

    public static CommonResult failure(Integer code, String message) {
        return new CommonResult(false, code, message, null);
    }


}
