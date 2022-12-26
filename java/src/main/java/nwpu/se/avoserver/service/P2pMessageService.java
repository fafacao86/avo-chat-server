package nwpu.se.avoserver.service;


import nwpu.se.avoserver.entity.P2pMessage;
import nwpu.se.avoserver.param.GetP2pMsgParam;
import nwpu.se.avoserver.param.SendP2pMsgParam;

import java.util.List;

/**
* @author xiaoheng
* @description 针对表【t_p2p_message(p2p聊天记录)】的数据库操作Service
* @createDate 2022-12-11 13:42:57
*/
public interface P2pMessageService  {

    List<P2pMessage> getP2pMessage(GetP2pMsgParam getP2pMsgParam);

    void sendP2pMessage(SendP2pMsgParam sendP2pMsgParam);
}
