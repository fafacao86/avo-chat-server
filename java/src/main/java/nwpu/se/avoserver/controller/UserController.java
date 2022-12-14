package nwpu.se.avoserver.controller;


import lombok.extern.slf4j.Slf4j;
import nwpu.se.avoserver.common.CommonUtil;
import nwpu.se.avoserver.common.JwtUtil;
import nwpu.se.avoserver.common.RedisUtil;
import nwpu.se.avoserver.constant.ResultCodeEnum;
import nwpu.se.avoserver.entity.User;
import nwpu.se.avoserver.exception.BusinessException;
import nwpu.se.avoserver.param.LoginParam;
import nwpu.se.avoserver.service.UserService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.validation.annotation.Validated;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RestController;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.validation.Valid;
import java.util.function.ObjLongConsumer;

@RestController
@Slf4j
public class UserController {
    @Autowired
    private UserService userService;

    @Autowired
    private JwtUtil jwtUtil;

    @Autowired
    private CommonUtil commonUtil;

    @PostMapping("/user/login")
    public Object login(@RequestBody @Valid LoginParam loginParam, HttpServletResponse response) {
        User user = userService.login(loginParam);
        String token = jwtUtil.getTokenFromUser(user);
        if(RedisUtil.KeyOps.hasKey(token)){
            throw new BusinessException(ResultCodeEnum.REPEAT_OPERATION, "用户已登录");
        }
        RedisUtil.HashOps.hPut(token, "login", "1");
        response.setHeader("token", token);
        return new Object();
    }

}
