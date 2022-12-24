package nwpu.se.avoserver.vo;

import lombok.Data;

import java.util.ArrayList;

@Data
public class ContactVO {
    private int contactOwnerID;
    private ArrayList<Integer> blackList;
    private ArrayList<Integer> contact;

    public ContactVO() {
        blackList = new ArrayList<>();
        contact = new ArrayList<>();
    }
}
