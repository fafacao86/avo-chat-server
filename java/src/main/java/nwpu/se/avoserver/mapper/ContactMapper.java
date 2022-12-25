package nwpu.se.avoserver.mapper;

import com.alibaba.fastjson2.JSONArray;
import nwpu.se.avoserver.entity.Contact;
import nwpu.se.avoserver.vo.ContactVO;
import org.apache.ibatis.annotations.Mapper;
import org.apache.ibatis.annotations.Param;

/**
* @author xiaoheng
* @description 针对表【t_contact(通讯录)】的数据库操作Mapper
* @createDate 2022-12-22 16:28:22
* @Entity nwpu.se.avoserver.entity.Contact
*/
@Mapper
public interface ContactMapper {

    Contact getContactById(@Param("userID") int userID);

    void updateContact(@Param("userID") int userID,@Param("contactList") String contactList);
}




