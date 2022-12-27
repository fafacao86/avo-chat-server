package nwpu.se.avoserver.mapper;


import nwpu.se.avoserver.entity.Group;
import org.apache.ibatis.annotations.Mapper;
import org.apache.ibatis.annotations.Param;
import org.springframework.security.core.parameters.P;

/**
* @author xiaoheng
* @description 针对表【t_group(群)】的数据库操作Mapper
* @createDate 2022-12-11 13:42:51
* @Entity nwpu.se.avoserver.entity.Group
*/
@Mapper
public interface GroupMapper {

    Group getGroupById(@Param("groupId") int groupId);

    void createGroup(@Param("groupId") int groupId,@Param("ownerId") Integer ownerId,@Param("groupName") String groupName,
                     @Param("members") String members);

    void modifyGroupName(@Param("groupId") int groupId, @Param("groupName") String groupName);

    void updateMembers(@Param("groupID") int groupID, @Param("members") String members);
}




