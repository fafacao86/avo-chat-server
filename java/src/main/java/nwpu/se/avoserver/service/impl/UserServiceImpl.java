package nwpu.se.avoserver.service.impl;

import nwpu.se.avoserver.common.CommonUtil;
import nwpu.se.avoserver.constant.ResultCodeEnum;
import nwpu.se.avoserver.entity.User;
import nwpu.se.avoserver.exception.BusinessException;
import nwpu.se.avoserver.param.LoginParam;
import nwpu.se.avoserver.param.RegisterParam;
import nwpu.se.avoserver.service.UserService;
import nwpu.se.avoserver.mapper.UserMapper;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.security.crypto.password.PasswordEncoder;
import org.springframework.stereotype.Service;

/**
* @author xiaoheng
* @description 针对表【t_user(user table)】的数据库操作Service实现
* @createDate 2022-12-11 13:42:00
*/
@Service
public class UserServiceImpl implements UserService{

    @Autowired
    private PasswordEncoder passwordEncoder;

    @Autowired
    private UserMapper userMapper;

    @Override
    public User login(LoginParam loginParam) {
        try {
            User user = userMapper.getUserById(loginParam.getID());
            if(passwordEncoder.matches(loginParam.getPassword(), user.getPassword())){
                return user;
            }else{
                throw new RuntimeException("密码错误");
            }
        }catch (Exception e){
            throw new BusinessException(ResultCodeEnum.WRONG_ID_OR_PASSWORD, "ID或密码错误");
        }
    }

    @Override
    public int register(RegisterParam registerParam) {
        try {
            User user = new User();
            user.setNickname(registerParam.getNickname());
            user.setPassword(passwordEncoder.encode(registerParam.getPassword()));
            int newUserID = CommonUtil.generateId();
            int result = userMapper.insertUser(newUserID, user.getNickname(), user.getPassword());
            if (result != 1) {
                throw new BusinessException(ResultCodeEnum.INTERNAL_ERROR,"注册失败");
            }
            return newUserID;
        }catch (BusinessException e){
            throw e;
        }
        catch (Exception e){
            throw new BusinessException(ResultCodeEnum.REPEAT_OPERATION, "用户已存在");
        }
    }
}




