package nwpu.se.avoserver.common;

import com.alibaba.fastjson.JSONObject;
import org.springframework.stereotype.Component;

import javax.servlet.http.HttpServletRequest;
import java.io.BufferedReader;
import java.io.IOException;
import java.sql.Timestamp;
import java.time.LocalDateTime;
import java.time.ZoneOffset;
import java.time.format.DateTimeFormatter;
import java.util.UUID;


@Component
public class CommonUtil {
    public static int generateId() {
        String id = " ";
        while(true){
            UUID uuid = UUID.randomUUID();
            // 使用正则表达式提取出UUID中的纯数字部分
            id = uuid.toString().replaceAll("[^\\d]", "");
            // 截取前8位作为ID, 如果不足8位或第一位是0则继续生成
            if(id.length() < 8 || id.charAt(0)=='0'){
                continue;
            }else {
                id = id.substring(0, 8);
                break;
            }
        }
       return Integer.parseInt(id);
    }

    public static Timestamp localDateTime2Timestamp(LocalDateTime localDateTime){
        return Timestamp.valueOf(localDateTime);
    }

    public static LocalDateTime timestamp2LocalDateTime(Timestamp timestamp){
        return timestamp.toLocalDateTime();
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
