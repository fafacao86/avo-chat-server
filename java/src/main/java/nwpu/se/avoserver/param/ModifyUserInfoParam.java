package nwpu.se.avoserver.param;


import lombok.Data;
import org.hibernate.validator.constraints.Length;

import javax.validation.constraints.AssertTrue;
import javax.validation.constraints.Max;
import javax.validation.constraints.Min;


/**
 * 修改用户信息参数
 * */
@Data
public class ModifyUserInfoParam {
    /**
     * nickname: 用户昵称
     * */
    @Length(min = 1, max = 20, message = "昵称长度必须在1-20之间")
    private String nickname;

    /**
     * sex: 性别
     * */
    @Max(2)
    @Min(1)
    private String sex;
}
