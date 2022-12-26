package nwpu.se.avoserver.mapper;


import nwpu.se.avoserver.entity.P2pMessage;
import nwpu.se.avoserver.vo.P2pMessageVO;
import org.apache.ibatis.annotations.Mapper;
import org.apache.ibatis.annotations.Param;

import java.time.LocalDateTime;
import java.util.List;
import java.util.Map;

/**
* @author xiaoheng
* @description 针对表【t_p2p_message(p2p聊天记录)】的数据库操作Mapper
* @createDate 2022-12-11 13:42:57
* @Entity nwpu.se.avoserver.entity.P2pMessage
*/
@Mapper
public interface P2pMessageMapper  {
     List<P2pMessage> selectByIdAndAfterTime(@Param("to_id") Integer receiverID, @Param("create_time") LocalDateTime after_time);

    int insertP2pMessage(@Param("from_id") Integer senderID, @Param("to_id") Integer receiverID, @Param("content") String content);
}




