package nwpu.se.avoserver;

import nwpu.se.avoserver.protocol.PipeOutputBean;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.CommandLineRunner;
import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.context.annotation.Import;
import org.springframework.security.crypto.bcrypt.BCryptPasswordEncoder;

import java.io.FileDescriptor;
import java.io.FileOutputStream;
import java.nio.file.attribute.FileAttribute;

@SpringBootApplication
public class AvoServerApplication implements CommandLineRunner {
    @Autowired
    private PipeOutputBean pipeOutputBean;

    public static void main(String[] args) {
        SpringApplication.run(AvoServerApplication.class, args);
    }

    @Override
    public void run(String... args) throws Exception {
        String pid = args[0];
        String fd = args[1];
        String fdPath = "/proc/" + pid + "/fd/" + fd;
        pipeOutputBean.setPipeOutput(new FileOutputStream(fdPath));
    }
}
