package nwpu.se.avoserver.constant;

/**
 * 用户的性别
 */
public enum UserSexEnum {
    /**
     * 男
     */
    MALE(1),
    /**
     * 女
     */
    FEMALE(2);
    private final int code;

    UserSexEnum(int code) {
        this.code = code;
    }

    public int getCode() {
        return code;
    }
}
