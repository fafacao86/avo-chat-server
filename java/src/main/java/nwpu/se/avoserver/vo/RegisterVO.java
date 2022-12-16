package nwpu.se.avoserver.vo;


import lombok.Data;

@Data
public class RegisterVO {
    private int userID;

    public RegisterVO(int userID) {
        this.userID = userID;
    }
}
