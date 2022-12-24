package nwpu.se.avoserver.param;


import lombok.Data;
import org.hibernate.validator.constraints.Length;

import javax.validation.constraints.Max;
import javax.validation.constraints.Min;
import javax.validation.constraints.NotBlank;
import javax.validation.constraints.NotNull;


/**
 * 获取用户信息参数
 * */
@Data
public class GetUserInfoParam {
    /**
     * ID: 用户ID
     * */
    @NotNull(message = "用户名不能为空")
    @Min(value = 10000000, message = "ID长度必须为8位")
    @Max(value = 99999999, message = "ID长度必须为8位")
    private int userID;
}
