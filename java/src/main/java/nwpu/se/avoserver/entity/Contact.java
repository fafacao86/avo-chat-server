package nwpu.se.avoserver.entity;

import java.io.Serializable;
import lombok.Data;

/**
 * 通讯录
 * @TableName t_contact
 */
@Data
public class Contact implements Serializable {
    /**
     * 通讯录所属者ID
     */
    private Integer userId;

    /**
     * 联系人数组
     */
    private String contactList;

    /**
     * 黑名单
     */
    private String blackList;

    private static final long serialVersionUID = 1L;

    @Override
    public boolean equals(Object that) {
        if (this == that) {
            return true;
        }
        if (that == null) {
            return false;
        }
        if (getClass() != that.getClass()) {
            return false;
        }
        Contact other = (Contact) that;
        return (this.getUserId() == null ? other.getUserId() == null : this.getUserId().equals(other.getUserId()))
            && (this.getContactList() == null ? other.getContactList() == null : this.getContactList().equals(other.getContactList()))
            && (this.getBlackList() == null ? other.getBlackList() == null : this.getBlackList().equals(other.getBlackList()));
    }

    @Override
    public int hashCode() {
        final int prime = 31;
        int result = 1;
        result = prime * result + ((getUserId() == null) ? 0 : getUserId().hashCode());
        result = prime * result + ((getContactList() == null) ? 0 : getContactList().hashCode());
        result = prime * result + ((getBlackList() == null) ? 0 : getBlackList().hashCode());
        return result;
    }

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();
        sb.append(getClass().getSimpleName());
        sb.append(" [");
        sb.append("Hash = ").append(hashCode());
        sb.append(", userId=").append(userId);
        sb.append(", contactList=").append(contactList);
        sb.append(", blackList=").append(blackList);
        sb.append(", serialVersionUID=").append(serialVersionUID);
        sb.append("]");
        return sb.toString();
    }
}