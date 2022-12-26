package nwpu.se.avoserver.vo;

import lombok.Data;
import lombok.NoArgsConstructor;

import java.util.ArrayList;

@Data
public class GroupVO {
    private String groupName;
    private int ownerID;
    private int groupID;
    private ArrayList<Integer> members;

    public GroupVO() {
        this.members = new ArrayList<>();
    }
}
