package nwpu.se.avoserver;

import nwpu.se.avoserver.protocol.PipeOutputBean;
import org.junit.jupiter.api.Test;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;

@SpringBootTest
class AvoServerApplicationTests {
    @Autowired
    private PipeOutputBean pipeOutputBean;

    @Test
    void testFileDescriptor() {
        System.out.println(pipeOutputBean.getPipeOutput());
    }

}
