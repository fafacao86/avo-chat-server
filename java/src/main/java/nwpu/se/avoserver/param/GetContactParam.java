package nwpu.se.avoserver.param;

import lombok.Data;

import javax.validation.constraints.Max;
import javax.validation.constraints.Min;
import javax.validation.constraints.NotBlank;
import javax.validation.constraints.NotNull;

@Data
public class GetContactParam {
    @NotNull(message = "用户名不能为空")
    @Max(value = 99999999,message = "ID长度必须为8位")
    @Min(value = 10000000,message = "ID长度必须为8位")
    private int userID;
}
