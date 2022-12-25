package nwpu.se.avoserver.service;

import nwpu.se.avoserver.entity.Contact;
import nwpu.se.avoserver.vo.ContactVO;

/**
* @author xiaoheng
* @description 针对表【t_contact(通讯录)】的数据库操作Service
* @createDate 2022-12-22 16:28:22
*/
public interface ContactService {

    ContactVO getContactById(int userID);

    Object modifyContact(Integer userId, int targetID, int operationType);
}
