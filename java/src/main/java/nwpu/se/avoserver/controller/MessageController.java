package nwpu.se.avoserver.controller;

import nwpu.se.avoserver.common.JwtUtil;
import nwpu.se.avoserver.constant.ResultCodeEnum;
import nwpu.se.avoserver.entity.P2pMessage;
import nwpu.se.avoserver.entity.User;
import nwpu.se.avoserver.exception.BusinessException;
import nwpu.se.avoserver.param.GetP2pMsgParam;
import nwpu.se.avoserver.param.SendP2pMsgParam;
import nwpu.se.avoserver.protocol.PipeOutputBean;
import nwpu.se.avoserver.service.P2pMessageService;
import nwpu.se.avoserver.vo.P2pMessageVO;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RestController;

import javax.servlet.http.HttpServletRequest;
import javax.validation.Valid;
import java.time.ZoneId;
import java.time.ZoneOffset;
import java.time.ZonedDateTime;
import java.util.ArrayList;
import java.util.List;

@RestController
public class MessageController {
    @Autowired
    private P2pMessageService p2pMessageService;
    @Autowired
    private JwtUtil jwtUtil;

    @Autowired
    private PipeOutputBean pipeOutputBean;


    @GetMapping("/api/message/p2p")
    public List<P2pMessageVO> getP2pMessage(@RequestBody @Valid GetP2pMsgParam getP2pMsgParam, HttpServletRequest request) {
        String token = request.getHeader("token");
        User requestIssuer = jwtUtil.getUserFromToken(token);
        System.out.println(requestIssuer);
        System.out.println(getP2pMsgParam);
        if(!getP2pMsgParam.getReceiverID().equals(requestIssuer.getUserId())){
            throw new BusinessException(ResultCodeEnum.REPEAT_OPERATION, "用户未登录");
        }
        List<P2pMessage>  p2pMessageList = p2pMessageService.getP2pMessage(getP2pMsgParam);
        List<P2pMessageVO> p2pMessageVOList = new ArrayList<>();
        for (P2pMessage p2pMessage : p2pMessageList) {
            P2pMessageVO p2pMessageVO = new P2pMessageVO();
            p2pMessageVO.setSenderID(p2pMessage.getFromId());
            p2pMessageVO.setReceiverID(p2pMessage.getToId());
            p2pMessageVO.setContent(p2pMessage.getContentJson());
            Long createTime = ZonedDateTime.of(p2pMessage.getCreateTime(), ZoneId.systemDefault()).toInstant().toEpochMilli();
            p2pMessageVO.setCreate_time(createTime);
            p2pMessageVOList.add(p2pMessageVO);
        }
        return p2pMessageVOList;
    }

    @PostMapping("/api/message/p2p")
    public Object sendP2pMessage(@RequestBody @Valid SendP2pMsgParam sendP2pMsgParam, HttpServletRequest request){
        String token = request.getHeader("token");
        User requestIssuer = jwtUtil.getUserFromToken(token);
        if(sendP2pMsgParam.getReceiverID()==requestIssuer.getUserId()){
            throw new BusinessException(ResultCodeEnum.REPEAT_OPERATION, "用户未登录");
        }
        p2pMessageService.sendP2pMessage(sendP2pMsgParam);
        pipeOutputBean.notifyP2P(sendP2pMsgParam.getReceiverID(), sendP2pMsgParam.getSenderID());
        return new Object();
    }
}
