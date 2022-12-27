package nwpu.se.avoserver.param;

import lombok.Data;
import org.hibernate.validator.constraints.Length;

import javax.validation.constraints.NotBlank;
import javax.validation.constraints.NotNull;

@Data
public class CreateGroupParam {

    @NotBlank(message = "群名不能为空")
    @NotNull(message = "群名不能为空")
    @Length(min = 1, max = 20, message = "群名长度必须在1～20之间")
    private String groupName;
}
