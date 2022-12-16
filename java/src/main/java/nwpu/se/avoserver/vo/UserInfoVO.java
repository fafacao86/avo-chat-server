package nwpu.se.avoserver.vo;

import lombok.Data;

@Data
public class UserInfoVO {
    private int userID;

    public UserInfoVO(int userID, String nickname, int sex) {
        this.userID = userID;
        this.nickname = nickname;
        this.sex = sex;
    }

    private String nickname;
    private int sex;


}
