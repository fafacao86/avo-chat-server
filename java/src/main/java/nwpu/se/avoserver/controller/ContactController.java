package nwpu.se.avoserver.controller;

import lombok.extern.slf4j.Slf4j;
import nwpu.se.avoserver.common.JwtUtil;
import nwpu.se.avoserver.entity.User;
import nwpu.se.avoserver.param.GetContactParam;
import nwpu.se.avoserver.param.ModifyContactParam;
import nwpu.se.avoserver.service.ContactService;
import nwpu.se.avoserver.vo.ContactVO;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RestController;

import javax.servlet.http.HttpServletRequest;
import javax.validation.Valid;

@RestController
@Slf4j
public class ContactController {
    @Autowired
    private ContactService contactService;

    @Autowired
    private JwtUtil jwtUtil;

    @GetMapping("/api/user/contact")
    public ContactVO getContactById(@RequestBody @Valid GetContactParam getContactParam){
        return contactService.getContactById(getContactParam.getUserID());
    }

    @PostMapping("/api/user/contact")
    public Object modifyContact(@RequestBody @Valid ModifyContactParam modifyContactParam,HttpServletRequest request){
        User user = jwtUtil.getUserFromToken(request.getHeader("token"));
        return contactService.modifyContact(user.getUserId(), modifyContactParam.getTargetID(),
                modifyContactParam.getOperationType());
    }
}
