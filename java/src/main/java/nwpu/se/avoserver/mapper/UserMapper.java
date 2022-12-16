package nwpu.se.avoserver.mapper;


import nwpu.se.avoserver.entity.User;
import org.apache.ibatis.annotations.Mapper;
import org.apache.ibatis.annotations.Param;

/**
* @author xiaoheng
* @description 针对表【t_user(user table)】的数据库操作Mapper
* @createDate 2022-12-11 13:42:00
* @Entity nwpu.se.avoserver.entity.User
*/
@Mapper
public interface UserMapper  {

    User getUserById(@Param("userID") String userID);
    int insertUser(@Param("userID") int userID, @Param("nickname") String nickname, @Param("password") String password);
}




