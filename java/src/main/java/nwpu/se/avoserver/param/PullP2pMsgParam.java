package nwpu.se.avoserver.param;


import lombok.Data;
import lombok.NonNull;
import org.hibernate.validator.constraints.Length;

import javax.validation.constraints.NotBlank;
import java.time.LocalDateTime;

/**
 * 拉取P2P消息参数
 * */
@Data
public class PullP2pMsgParam {
/**
     * senderID: 发送者ID
     * */
    @NotBlank
    @Length(min = 8, max = 8, message = "ID长度必须为8位")
    private String senderID;


    /**
     * receiverID: 接收者ID
     * */
    @NotBlank
    @Length(min = 8, max = 8, message = "ID长度必须为8位")
    private String receiverID;


    /**
     * after_time: 自多久以来的消息
     * */
    @NonNull
    private  int after_time;
}
