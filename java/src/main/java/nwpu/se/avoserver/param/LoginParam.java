package nwpu.se.avoserver.param;

import io.lettuce.core.dynamic.annotation.Key;
import lombok.Data;
import lombok.NonNull;
import org.hibernate.validator.constraints.Length;

import javax.validation.constraints.NotBlank;

/**
 * 登录参数
 * */
@Data
public class LoginParam {
    /**
     * ID: 用户ID
     * */
    @NotBlank(message = "用户名不能为空")
    @Length(min = 8, max = 8, message = "ID长度必须为8位")
    private String ID;


    /**
     * password: 密码
     * */
    @NotBlank(message = "密码不能为空")
    @Length(min = 8, max = 8, message = "ID长度必须为8位")
    private String password;
}
