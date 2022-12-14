package nwpu.se.avoserver.protocol;


import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;

import java.io.FileOutputStream;

@Configuration
public class PipeConfig {
    @Bean
    public PipeOutputBean PipeOutput() {
        return new PipeOutputBean();
    }
}
