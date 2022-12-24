package nwpu.se.avoserver.service.impl;

import com.alibaba.fastjson2.JSONArray;
import nwpu.se.avoserver.constant.ResultCodeEnum;
import nwpu.se.avoserver.entity.Contact;
import nwpu.se.avoserver.exception.BusinessException;
import nwpu.se.avoserver.service.ContactService;
import nwpu.se.avoserver.mapper.ContactMapper;
import nwpu.se.avoserver.vo.ContactVO;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;


/**
* @author xiaoheng
* @description 针对表【t_contact(通讯录)】的数据库操作Service实现
* @createDate 2022-12-22 16:28:22
*/
@Service
public class ContactServiceImpl implements ContactService{
    @Autowired
    private ContactMapper contactMapper;

    @Override
    public ContactVO getContactById(int userID) {
        try {
            Contact contact = contactMapper.getContactById(userID);
            if (contact == null){
                throw new BusinessException(ResultCodeEnum.INTERNAL_ERROR, "用户通讯录不存在");
            }
            ContactVO contactVO = new ContactVO();
            contactVO.setContactOwnerID(contact.getUserId());

            JSONArray contactList = JSONArray.parseArray(contact.getContactList());
            if (contactList != null) {
                for (Object o:contactList){
                    contactVO.getContact().add((Integer) o);
                }
            }

            JSONArray blackList = JSONArray.parseArray(contact.getBlackList());
            if (blackList != null){
                for (Object o:blackList){
                    contactVO.getBlackList().add((Integer) o);
                }
            }

            return contactVO;
        }catch (Exception e){
            throw new BusinessException(ResultCodeEnum.INTERNAL_ERROR, e.getMessage());
        }

    }

    @Override
    public Object modifyContact(Integer userId, int targetID, int operationType) {
        try {
            Contact contact = contactMapper.getContactById(userId);
            JSONArray contactList = JSONArray.parseArray(contact.getContactList());
            if (operationType == 1){
                contactList.remove((Integer)targetID);
                contactMapper.updateContact(userId,JSONArray.toJSONString(contactList));
            }else if (operationType == 2){
                contactList.add((Integer)targetID);
                contactMapper.updateContact(userId,JSONArray.toJSONString(contactList));
            }
            return new Object();
        }catch (Exception e){
            throw new BusinessException(ResultCodeEnum.INTERNAL_ERROR,e.getMessage());
        }
    }
}




