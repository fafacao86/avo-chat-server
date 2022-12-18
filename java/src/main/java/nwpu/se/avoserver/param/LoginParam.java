package nwpu.se.avoserver.param;

import io.lettuce.core.dynamic.annotation.Key;
import lombok.Data;
import lombok.NonNull;
import org.hibernate.validator.constraints.Length;

import javax.validation.constraints.NotBlank;
import javax.validation.constraints.NotNull;

/**
 * 登录参数
 * */
@Data
public class LoginParam {
    /**
     * ID: 用户ID
     * */
    @NotNull
    @Length(min = 8, max = 8, message = "ID长度必须为8位")
    private String userID;


    /**
     * password: 密码
     * */
    @NotBlank(message = "密码不能为空")
    @Length(min = 6, max = 20, message = "密码长度为6-20位")
    private String password;
}
