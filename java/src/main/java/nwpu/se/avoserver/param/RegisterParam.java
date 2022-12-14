package nwpu.se.avoserver.param;

import lombok.Data;
import org.apache.ibatis.annotations.Mapper;
import org.hibernate.validator.constraints.Length;


/**
 * 注册参数
 * */
@Data
public class RegisterParam {
    /**
     * nickname: 用户昵称
     * */
    @Length(min = 1, max = 20, message = "昵称长度必须在1-20之间")
    private String nickname;

    /**
     * password: 密码
     * */
    @Length(min = 6, max = 20, message = "密码长度必须在6-20之间")
    private String password;
}
