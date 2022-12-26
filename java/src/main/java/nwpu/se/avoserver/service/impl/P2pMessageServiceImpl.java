package nwpu.se.avoserver.service.impl;

import nwpu.se.avoserver.constant.ResultCodeEnum;
import nwpu.se.avoserver.entity.P2gMessage;
import nwpu.se.avoserver.entity.P2pMessage;
import nwpu.se.avoserver.exception.BusinessException;
import nwpu.se.avoserver.param.GetP2pMsgParam;
import nwpu.se.avoserver.param.SendP2pMsgParam;
import nwpu.se.avoserver.service.P2pMessageService;
import nwpu.se.avoserver.mapper.P2pMessageMapper;
import nwpu.se.avoserver.vo.P2pMessageVO;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import javax.validation.constraints.Max;
import java.sql.Timestamp;
import java.time.LocalDateTime;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;

/**
* @author xiaoheng
* @description 针对表【t_p2p_message(p2p聊天记录)】的数据库操作Service实现
* @createDate 2022-12-11 13:42:57
*/
@Service
public class P2pMessageServiceImpl implements P2pMessageService{
    @Autowired
    private P2pMessageMapper p2pMessageMapper;

    @Override
    public List<P2pMessage> getP2pMessage(GetP2pMsgParam getP2pMsgParam) {
        Timestamp after_time = new Timestamp(getP2pMsgParam.getAfter_time());
        LocalDateTime localDateTime = after_time.toLocalDateTime();
        System.out.println(after_time);
        System.out.println(getP2pMsgParam.getReceiverID());
        System.out.println(getP2pMsgParam.getSenderID());
        List<P2pMessage> p2pMessageList = p2pMessageMapper.selectByIdAndAfterTime(getP2pMsgParam.getReceiverID(), localDateTime);
        if (p2pMessageList == null) {
            throw new BusinessException(ResultCodeEnum.INTERNAL_ERROR, "查找消息失败");
        }
        return p2pMessageList;
    }

    @Override
    public void sendP2pMessage(SendP2pMsgParam sendP2pMsgParam) {
        int result = p2pMessageMapper.insertP2pMessage(sendP2pMsgParam.getSenderID(), sendP2pMsgParam.getReceiverID(), sendP2pMsgParam.getContent());
        if (result != 1) {
            throw new BusinessException(ResultCodeEnum.INTERNAL_ERROR, "发送消息失败");
        }
    }
}




