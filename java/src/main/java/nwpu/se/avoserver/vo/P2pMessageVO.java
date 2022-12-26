package nwpu.se.avoserver.vo;

import lombok.Data;

@Data
public class P2pMessageVO {
    private Integer senderID;
    private Integer receiverID;
    private Long create_time;
    private String content;
}
