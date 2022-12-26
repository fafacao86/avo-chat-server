package nwpu.se.avoserver.param;


import lombok.Data;
import lombok.NonNull;
import org.hibernate.validator.constraints.Length;

import javax.validation.constraints.Max;
import javax.validation.constraints.Min;
import javax.validation.constraints.NotBlank;

/**
 * 拉取P2P消息参数
 * */
@Data
public class GetP2pMsgParam {
/**
     * senderID: 发送者ID
     * */
    @Min(value = 10000000, message = "ID长度必须为8位")
    @Max(value = 99999999, message = "ID长度必须为8位")
    private Integer senderID;


    /**
     * receiverID: 接收者ID
     * */
    @Min(value = 10000000, message = "ID长度必须为8位")
    @Max(value = 99999999, message = "ID长度必须为8位")
    private Integer receiverID;


    /**
     * after_time: 自多久以来的消息
     * */
    private long after_time;
}
