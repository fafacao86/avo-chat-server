package nwpu.se.avoserver.common;

import com.alibaba.fastjson.JSONObject;
import org.springframework.stereotype.Component;

import javax.servlet.http.HttpServletRequest;
import java.io.BufferedReader;
import java.io.IOException;
import java.time.LocalDateTime;
import java.time.ZoneOffset;
import java.time.format.DateTimeFormatter;
import java.util.UUID;


@Component
public class CommonUtil {
    public static String generateId() {
        String id = " ";
        while(true){
            UUID uuid = UUID.randomUUID();
            // 使用正则表达式提取出UUID中的纯数字部分
            id = uuid.toString().replaceAll("[^\\d]", "");
            // 截取前8位作为ID
            if(id.length() < 8){
                continue;
            }else {
                id = id.substring(0, 8);
                break;
            }
        }
       return id;
    }

    public static int localDateTime2int(LocalDateTime localDateTime){
         long seconds= localDateTime.toEpochSecond(ZoneOffset.of("+8"));
            return (int)seconds;
    }

    public static LocalDateTime int2LocalDateTime(int timestampInSeconds){
        return LocalDateTime.ofEpochSecond(timestampInSeconds, 0, ZoneOffset.ofHours(8));
    }

    public static String ReadHttpBodyAsString(HttpServletRequest request)
    {

        BufferedReader br = null;
        StringBuilder sb = new StringBuilder("");
        try
        {
            br = request.getReader();
            String str;
            while ((str = br.readLine()) != null)
            {
                sb.append(str);
            }
            br.close();
        }
        catch (IOException e)
        {
            e.printStackTrace();
        }
        finally
        {
            if (null != br)
            {
                try
                {
                    br.close();
                }
                catch (IOException e)
                {
                    e.printStackTrace();
                }
            }
        }
        return sb.toString();
    }

    public static String getStringFieldFromJson(String jsonStr, String field){
        JSONObject jsonObj = JSONObject.parseObject(jsonStr);
        return jsonObj.getString(field);
    }
}
