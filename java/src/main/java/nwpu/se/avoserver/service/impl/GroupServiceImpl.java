package nwpu.se.avoserver.service.impl;

import com.alibaba.fastjson2.JSONArray;
import nwpu.se.avoserver.common.CommonUtil;
import nwpu.se.avoserver.constant.ResultCodeEnum;
import nwpu.se.avoserver.entity.Group;
import nwpu.se.avoserver.exception.BusinessException;
import nwpu.se.avoserver.service.GroupService;
import nwpu.se.avoserver.mapper.GroupMapper;
import nwpu.se.avoserver.vo.GroupVO;
import nwpu.se.avoserver.vo.IsGroupVO;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;


/**
* @author xiaoheng
* @description 针对表【t_group(群)】的数据库操作Service实现
* @createDate 2022-12-11 13:42:51
*/
@Service
public class GroupServiceImpl implements GroupService{

    @Autowired
    private GroupMapper groupMapper;

    @Override
    public Object createGroup(Integer userId, String groupName) {
        try {
            int groupId = CommonUtil.generateId();
            while (groupMapper.getGroupById(groupId) != null){
                groupId = CommonUtil.generateId();
            }
            StringBuilder sb = new StringBuilder();
            sb.append("[").append(userId).append("]");
            String members = sb.toString();
            groupMapper.createGroup(groupId,userId,groupName,members);
            return new Object();
        }catch (Exception e){
            throw new BusinessException(ResultCodeEnum.INTERNAL_ERROR, e.getMessage());
        }

    }

    @Override
    public Object modifyGroupName(int groupId, String groupName) {
        try {
            if (groupMapper.getGroupById(groupId) == null){
                throw new BusinessException(ResultCodeEnum.INTERNAL_ERROR, "要修改的群聊不存在");
            }
            groupMapper.modifyGroupName(groupId, groupName);
            return new Object();
        }catch (Exception e){
            throw new BusinessException(ResultCodeEnum.INTERNAL_ERROR, e.getMessage());
        }
    }

    @Override
    public GroupVO getGroupInfo(int groupID) {
        try {
            Group group = groupMapper.getGroupById(groupID);
            if (group == null){
                throw new BusinessException(ResultCodeEnum.INTERNAL_ERROR, "群不存在");
            }
            GroupVO groupVO = new GroupVO();
            groupVO.setGroupID(groupID);
            groupVO.setGroupName(group.getGroupName());
            groupVO.setOwnerID(group.getOwnerId());
            String members = group.getMembers();
            JSONArray ja = JSONArray.parseArray(members);
            for (Object o:ja){
                groupVO.getMembers().add((Integer) o);
            }
            return groupVO;
        }catch (Exception e){
            throw new BusinessException(ResultCodeEnum.INTERNAL_ERROR,e.getMessage());
        }
    }

    @Override
    public void joinInGroup(int groupID, Integer userId) {
        try {
            Group group = groupMapper.getGroupById(groupID);
            if (group == null){
                throw new BusinessException(ResultCodeEnum.INTERNAL_ERROR, "要加入的群聊不存在");
            }
            JSONArray ja = JSONArray.parseArray(group.getMembers());
            if (ja.contains(userId)){
                throw new BusinessException(ResultCodeEnum.INTERNAL_ERROR, userId + "已在群聊中");
            }else {
                ja.add(userId);
                groupMapper.updateMembers(groupID, JSONArray.toJSONString(ja));
            }
        }catch (Exception e){
            throw new BusinessException(ResultCodeEnum.INTERNAL_ERROR, e.getMessage());
        }
    }

    @Override
    public void quitGroup(int groupID, Integer userId) {
        try {
            Group group = groupMapper.getGroupById(groupID);
            if (group == null){
                throw new BusinessException(ResultCodeEnum.INTERNAL_ERROR, "群ID有误，群不存在");
            }
            JSONArray ja = JSONArray.parseArray(group.getMembers());
            if (ja.contains(userId)){
                ja.remove(userId);
                groupMapper.updateMembers(groupID, JSONArray.toJSONString(ja));
            }else {
                throw new BusinessException(ResultCodeEnum.INTERNAL_ERROR, userId + "不在群聊中");
            }
        }catch (Exception e){
            throw new BusinessException(ResultCodeEnum.INTERNAL_ERROR, e.getMessage());
        }
    }

    @Override
    public IsGroupVO isGroup(int groupID) {
        try {
            Group group = groupMapper.getGroupById(groupID);
            if (group != null){
                return new IsGroupVO(true);
            }else {
                return new IsGroupVO(false);
            }
        }catch (Exception e){
            throw new BusinessException(ResultCodeEnum.INTERNAL_ERROR, e.getMessage());
        }
    }
}




