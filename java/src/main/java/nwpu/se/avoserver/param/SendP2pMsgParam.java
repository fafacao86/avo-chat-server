package nwpu.se.avoserver.param;


import lombok.Data;
import lombok.NonNull;
import org.hibernate.validator.constraints.Length;


/**
 * 发送P2P消息
 * */
@Data
public class SendP2pMsgParam {
    /**
     * senderID: 发送者ID
     * */
    @Length(min = 8, max = 8, message = "ID长度必须为8位")
    private String senderID;

    /**
     * receiverID: 接受者ID
     * */
    @Length(min = 8, max = 8, message = "ID长度必须为8位")
    private String receiverID;

    /**
     * content: 消息内容
     * */
    @NonNull
    private String content;
}
