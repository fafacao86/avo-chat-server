package nwpu.se.avoserver.param;

import lombok.Data;

import javax.validation.constraints.Max;
import javax.validation.constraints.Min;
import javax.validation.constraints.NotNull;

@Data
public class QuitGroupParam {

    @NotNull
    @Max(value = 99999999,message = "群ID必须为8位数字")
    @Min(value = 10000000,message = "群ID必须为8位数字")
    private int groupID;
}
