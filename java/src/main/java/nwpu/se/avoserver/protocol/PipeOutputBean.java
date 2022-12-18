package nwpu.se.avoserver.protocol;



import com.alibaba.fastjson.JSONObject;
import lombok.extern.slf4j.Slf4j;
import nwpu.se.avoserver.common.RedisUtil;

import java.io.FileOutputStream;

@Slf4j
public class PipeOutputBean {
    private FileOutputStream pipeOutput;

    public void setPipeOutput(FileOutputStream pipeOutput) {
        this.pipeOutput = pipeOutput;
    }

    public FileOutputStream getPipeOutput() {
        return pipeOutput;
    }

    public PipeOutputBean() {
    }

    public Boolean notifyP2P(Integer puller, Integer pull_target){
        try{
            JSONObject jsonObject = new JSONObject();
            jsonObject.put("type", 1); // 1表示p2p，对应T_P2P
            jsonObject.put("puller", puller.toString());
            jsonObject.put("pull_target", pull_target.toString());
            int jsonLength = jsonObject.toJSONString().length();
            String jsonLengthString = String.format("%04d", jsonLength);
            String json = "0"+jsonLengthString + jsonObject.toJSONString();
            pipeOutput.write(json.getBytes());
            pipeOutput.flush();
            return true;
        }catch (Exception e){
            log.warn("PipeOutputBean.notifyP2P error: " + e.getMessage());
            return false;
        }
    }

    public Boolean logOut(String token){
        try{

            String heartbeat_fd =  (String) RedisUtil.HashOps.hGet(token, "1"); // 1代表heartbeat_fd
            JSONObject jsonObject = new JSONObject();
            jsonObject.put("heartbeat_fd", heartbeat_fd);
            int jsonLength = jsonObject.toJSONString().length();
            String jsonLengthString = String.format("%04d", jsonLength);
            String json = jsonLengthString + jsonObject.toJSONString();
            pipeOutput.write(json.getBytes());
            pipeOutput.flush();
            return true;
        }catch (Exception e){
            log.warn("PipeOutputBean.logOut error: " + e.getMessage());
            return false;
        }
    }


}
