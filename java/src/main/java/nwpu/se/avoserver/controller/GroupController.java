package nwpu.se.avoserver.controller;

import lombok.extern.slf4j.Slf4j;
import nwpu.se.avoserver.common.JwtUtil;
import nwpu.se.avoserver.entity.User;
import nwpu.se.avoserver.param.*;
import nwpu.se.avoserver.service.GroupService;
import nwpu.se.avoserver.vo.GroupVO;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RestController;

import javax.servlet.http.HttpServletRequest;
import javax.validation.Valid;

@RestController
@Slf4j
public class GroupController {

    @Autowired
    private GroupService groupService;

    @Autowired
    private JwtUtil jwtUtil;

    @PostMapping("/group/create")
    public Object createGroup(@RequestBody @Valid CreateGroupParam createGroupParam, HttpServletRequest request){
        User user = jwtUtil.getUserFromToken(request.getHeader("token"));
        return groupService.createGroup(user.getUserId(), createGroupParam.getGroupName());
    }

    @PostMapping("/group/name")
    public Object modifyGroupName(@RequestBody @Valid ModifyGroupNameParam modifyGroupNameParam){
        return groupService.modifyGroupName(modifyGroupNameParam.getGroupID(), modifyGroupNameParam.getGroupName());
    }

    @GetMapping("/group/info")
    public GroupVO getGroupInfo(@RequestBody @Valid GetGroupInfoParam getGroupInfoParam){
        return groupService.getGroupInfo(getGroupInfoParam.getGroupID());
    }

    @PostMapping("/group/join")
    public Object joinInGroup(@RequestBody @Valid JoinInGroupParam joinInGroupParam, HttpServletRequest request){
        User user = jwtUtil.getUserFromToken(request.getHeader("token"));
        return groupService.joinInGroup(joinInGroupParam.getGroupID(), user.getUserId());
    }

    @PostMapping("/group/quit")
    public Object quitGroup(@RequestBody @Valid QuitGroupParam quitGroupParam, HttpServletRequest request){
        User user = jwtUtil.getUserFromToken(request.getHeader("token"));
        return groupService.quitGroup(quitGroupParam.getGroupID(), user.getUserId());
    }
}
