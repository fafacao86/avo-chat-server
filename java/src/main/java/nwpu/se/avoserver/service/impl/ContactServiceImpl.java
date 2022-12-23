package nwpu.se.avoserver.service.impl;

import com.baomidou.mybatisplus.extension.service.impl.ServiceImpl;
import nwpu.se.avoserver.entity.Contact;
import nwpu.se.avoserver.service.ContactService;
import nwpu.se.avoserver.mapper.ContactMapper;
import org.springframework.stereotype.Service;

/**
* @author xiaoheng
* @description 针对表【t_contact(通讯录)】的数据库操作Service实现
* @createDate 2022-12-22 16:28:22
*/
@Service
public class ContactServiceImpl extends ServiceImpl<ContactMapper, Contact>
    implements ContactService{

}




