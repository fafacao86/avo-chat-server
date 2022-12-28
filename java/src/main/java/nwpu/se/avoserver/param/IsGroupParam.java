package nwpu.se.avoserver.param;

import lombok.Data;

import javax.validation.constraints.Max;
import javax.validation.constraints.Min;
import javax.validation.constraints.NotNull;

@Data
public class IsGroupParam {

    @NotNull(message = "群ID不能为空")
    @Max(value = 99999999,message = "群ID必须为8为数字")
    @Min(value = 10000000,message = "群ID必须为8为数字")
    private int targetID;
}
