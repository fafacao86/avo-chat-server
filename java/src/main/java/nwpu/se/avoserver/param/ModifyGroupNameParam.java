package nwpu.se.avoserver.param;

import lombok.Data;
import org.hibernate.validator.constraints.Length;

import javax.validation.constraints.Max;
import javax.validation.constraints.Min;
import javax.validation.constraints.NotBlank;
import javax.validation.constraints.NotNull;

@Data
public class ModifyGroupNameParam {

    @NotNull(message = "用户名不能为空")
    @Max(value = 99999999, message = "群ID长度必须为8位")
    @Min(value = 10000000,message = "群ID长度必须为8位")
    private int groupID;

    @NotBlank(message = "群名不能为空")
    @NotNull(message = "群名不能为空")
    @Length(min = 1, max = 20, message = "群名长度应在1～20之间")
    private String groupName;
}
