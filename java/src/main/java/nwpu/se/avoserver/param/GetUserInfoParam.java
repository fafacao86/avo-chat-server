package nwpu.se.avoserver.param;


import lombok.Data;
import org.hibernate.validator.constraints.Length;

import javax.validation.constraints.NotBlank;


/**
 * 获取用户信息参数
 * */
@Data
public class GetUserInfoParam {
    /**
     * ID: 用户ID
     * */
    @NotBlank(message = "用户名不能为空")
    @Length(min = 8, max = 8, message = "ID长度必须为8位")
    private String ID;
}
