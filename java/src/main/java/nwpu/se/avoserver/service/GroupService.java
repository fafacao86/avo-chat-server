package nwpu.se.avoserver.service;


import nwpu.se.avoserver.vo.GroupVO;
import nwpu.se.avoserver.vo.IsGroupVO;

/**
* @author xiaoheng
* @description 针对表【t_group(群)】的数据库操作Service
* @createDate 2022-12-11 13:42:51
*/
public interface GroupService {

    Object createGroup(Integer userId, String groupName);

    Object modifyGroupName(int groupId, String groupName);

    GroupVO getGroupInfo(int groupID);

    void joinInGroup(int groupID, Integer userId);

    void quitGroup(int groupID, Integer userId);

    IsGroupVO isGroup(int groupID);
}
